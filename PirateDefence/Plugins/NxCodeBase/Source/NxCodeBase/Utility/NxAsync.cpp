// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxAsync.h"
#include "Containers/Ticker.h"
#include "Engine/AssetManager.h"
#include "Stats/Stats.h"
#include "NxBaseLog.h"

TMap<FNxAsync*, TSharedRef<FNxAsync::FNxLoadingState>> FNxAsync::Loading;

FNxAsync::FNxAsync()
{
}

FNxAsync::~FNxAsync()
{
	check(IsInGameThread());

	// Removing the loading state will cancel any pending loadings it was 
	// monitoring, and shouldn't receive any future callbacks for completion.
	Loading.Remove(this);
}

const FNxAsync::FNxLoadingState& FNxAsync::GetLoadingStateConst() const
{
	check(IsInGameThread());

	return Loading.FindChecked(this).Get();
}

FNxAsync::FNxLoadingState& FNxAsync::GetLoadingState()
{
	check(IsInGameThread());

	if (TSharedRef<FNxLoadingState>* LoadingState = Loading.Find(this))
	{
		return (*LoadingState).Get();
	}

	return Loading.Add(this, MakeShared<FNxLoadingState>(*this)).Get();
}

bool FNxAsync::HasLoadingState() const
{
	check(IsInGameThread());

	return Loading.Contains(this);
}

void FNxAsync::CancelAsyncLoading()
{
	// Don't create the loading state if we don't have anything pending.
	if (HasLoadingState())
	{
		GetLoadingState().CancelAndDestroy();
	}
}

bool FNxAsync::IsAsyncLoadingInProgress() const
{
	// Don't create the loading state if we don't have anything pending.
	if (HasLoadingState())
	{
		return GetLoadingStateConst().IsLoadingInProgress();
	}

	return false;
}

bool FNxAsync::IsLoadingInProgressOrPending() const
{
	if (HasLoadingState())
	{
		return GetLoadingStateConst().IsLoadingInProgressOrPending();
	}

	return false;
}

void FNxAsync::AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncLoad(SoftObjectPath, DelegateToCall);
}

void FNxAsync::AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncLoad(SoftObjectPaths, DelegateToCall);
}

void FNxAsync::AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, DelegateToCall);
}

void FNxAsync::AsyncCondition(TSharedRef<FNxAsyncCondition> Condition, const FSimpleDelegate& Callback)
{
	GetLoadingState().AsyncCondition(Condition, Callback);
}

void FNxAsync::AsyncEvent(const FSimpleDelegate& Callback)
{
	GetLoadingState().AsyncEvent(Callback);
}

void FNxAsync::StartAsyncLoading()
{
	// If we don't actually have any loading state because they've not queued anything to load,
	// just immediately start and finish the operation by calling the callbacks, no point in allocating
	// the memory just to de-allocate it.
	if (IsLoadingInProgressOrPending())
	{
		GetLoadingState().Start();
	}
	else
	{
		OnStartedLoading();
		OnFinishedLoading();
	}
}

//------------------------------------------------------------------------------

FNxAsync::FNxLoadingState::FNxLoadingState(FNxAsync& InOwner) : OwnerRef(InOwner)
{
}

FNxAsync::FNxLoadingState::~FNxLoadingState()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncMixin_FLoadingState_DestroyThisMemoryDelegate);

	NxPrintVerbose(LogAsset, TEXT("[0x%X] Destroy LoadingState (Done)"), this);

	// If we get destroyed, need to cancel whatever we're doing and cancel any
	// pending destruction - as we're already on the way out.
	CancelOnly(/*bDestroying*/true);
	CancelDestroyThisMemory(/*bDestroying*/true);
}

void FNxAsync::FNxLoadingState::CancelOnly(bool bDestroying)
{
	if (!bDestroying)
	{
		NxPrintVerbose(LogAsset, TEXT("[0x%X] Cancel"), this);
	}

	CancelStartTimer();

	for (TUniquePtr<FNxAsyncStep>& Step : AsyncSteps)
	{
		Step->Cancel();
	}

	// Moving the memory to another array so we don't crash.
	// There was an issue where the Step would get corrupted because we were calling Reset() on the array.
	AsyncStepsPendingDestruction = MoveTemp(AsyncSteps);

	bPreloadedBundles = false;
	bHasStarted = false;
	CurrentAsyncStep = 0;
}

void FNxAsync::FNxLoadingState::CancelAndDestroy()
{
	CancelOnly(/*bDestroying*/false);
	RequestDestroyThisMemory();
}

void FNxAsync::FNxLoadingState::CancelDestroyThisMemory(bool bDestroying)
{
	// If we've schedule the memory to be deleted we need to abort that.
	if (IsPendingDestroy())
	{
		if (!bDestroying)
		{
			NxPrintVerbose(LogAsset, TEXT("[0x%X] Destroy LoadingState (Canceled)"), this);
		}

		FTSTicker::GetCoreTicker().RemoveTicker(DestroyMemoryDelegate);
		DestroyMemoryDelegate.Reset();
	}
}

void FNxAsync::FNxLoadingState::RequestDestroyThisMemory()
{
	// If we're already pending to destroy this memory, just ignore.
	if (!IsPendingDestroy())
	{
		NxPrintVerbose(LogAsset, TEXT("[0x%X] Destroy LoadingState (Requested)"), this);

		DestroyMemoryDelegate = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) {
			// Remove any memory we were using.
			FNxAsync::Loading.Remove(&OwnerRef);
			return false;
		}));
	}
}

void FNxAsync::FNxLoadingState::CancelStartTimer()
{
	if (StartTimerDelegate.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(StartTimerDelegate);
		StartTimerDelegate.Reset();
	}
}

void FNxAsync::FNxLoadingState::Start()
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X] Start (Current Progress %d/%d)"), this, CurrentAsyncStep + 1, AsyncSteps.Num());

	// Cancel any pending kickoff load requests.
	CancelStartTimer();

	bool bStartingStepFound = false;

	if (!bHasStarted)
	{
		bHasStarted = true;
		OwnerRef.OnStartedLoading();
	}
	
	TryCompleteAsyncLoading();
}

void FNxAsync::FNxLoadingState::AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& DelegateToCall)
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X] AsyncLoad '%s'"), this, *SoftObjectPath.ToString());

	AsyncSteps.Add(
		MakeUnique<FNxAsyncStep>(
			DelegateToCall,
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftObjectPath, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT("VxAsync"))
			)
	);

	TryScheduleStart();
}

void FNxAsync::FNxLoadingState::AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall)
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X] AsyncLoad [%s]"), this, *FString::JoinBy(SoftObjectPaths, TEXT(", "), [](const FSoftObjectPath& SoftObjectPath) { return FString::Printf(TEXT("'%s'"), *SoftObjectPath.ToString()); }));

	AsyncSteps.Add(
		MakeUnique<FNxAsyncStep>(
			DelegateToCall,
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftObjectPaths, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT("VxAsync"))
			)
	);

	TryScheduleStart();
}

void FNxAsync::FNxLoadingState::AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall)
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X]  AsyncPreload Assets [%s], Bundles[%s]"),
		this,
		*FString::JoinBy(AssetIds, TEXT(", "), [](const FPrimaryAssetId& AssetId) { return *AssetId.ToString(); }),
		*FString::JoinBy(LoadBundles, TEXT(", "), [](const FName& LoadBundle) { return *LoadBundle.ToString(); })
	);

	TSharedPtr<FStreamableHandle> StreamingHandle;

	if (AssetIds.Num() > 0)
	{
		bPreloadedBundles = true;

		const bool bLoadRecursive = true;
		StreamingHandle = UAssetManager::Get().PreloadPrimaryAssets(AssetIds, LoadBundles, bLoadRecursive);
	}

	AsyncSteps.Add(MakeUnique<FNxAsyncStep>(DelegateToCall, StreamingHandle));

	TryScheduleStart();
}

void FNxAsync::FNxLoadingState::AsyncCondition(TSharedRef<FNxAsyncCondition> Condition, const FSimpleDelegate& DelegateToCall)
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X] AsyncCondition '0x%X'"), this, &Condition.Get());

	AsyncSteps.Add(MakeUnique<FNxAsyncStep>(DelegateToCall, Condition));

	TryScheduleStart();
}

void FNxAsync::FNxLoadingState::AsyncEvent(const FSimpleDelegate& DelegateToCall)
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X] AsyncEvent"), this);

	AsyncSteps.Add(MakeUnique<FNxAsyncStep>(DelegateToCall));

	TryScheduleStart();
}

void FNxAsync::FNxLoadingState::TryScheduleStart()
{
	CancelDestroyThisMemory(/*bDestroying*/false);

	// In the event the user forgets to start async loading, we'll begin doing it next frame.
	if (!StartTimerDelegate.IsValid())
	{
		StartTimerDelegate = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) {
			QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncMixin_FLoadingState_TryScheduleStartDelegate);
			Start();
			return false;
		}));
	}
}

bool FNxAsync::FNxLoadingState::IsLoadingInProgress() const
{
	if (AsyncSteps.Num() > 0)
	{
		if (CurrentAsyncStep < AsyncSteps.Num())
		{
			if (CurrentAsyncStep == (AsyncSteps.Num() - 1))
			{
				return AsyncSteps[CurrentAsyncStep]->IsLoadingInProgress();
			}

			// If we know it's a valid index, but not the last one, then we know we're still loading,
			// if it's not a valid index, we know there's no loading, or we're beyond any loading.
			return true;
		}
	}

	return false;
}

bool FNxAsync::FNxLoadingState::IsLoadingInProgressOrPending() const
{
	return StartTimerDelegate.IsValid() || IsLoadingInProgress();
}

bool FNxAsync::FNxLoadingState::IsPendingDestroy() const
{
	return DestroyMemoryDelegate.IsValid();
}

void FNxAsync::FNxLoadingState::TryCompleteAsyncLoading()
{
	// If we haven't started when we get this callback it means we've already completed
	// and this is some other callback finishing on the same frame/stack that we need to avoid
	// doing anything with until the memory is finished being deleted.
	if (!bHasStarted)
	{
		return;
	}

	NxPrintVerbose(LogAsset, TEXT("[0x%X] TryCompleteAsyncLoading - (Current Progress %d/%d)"), this, CurrentAsyncStep + 1, AsyncSteps.Num());

	while (CurrentAsyncStep < AsyncSteps.Num())
	{
		FNxAsyncStep* Step = AsyncSteps[CurrentAsyncStep].Get();
		if (Step->IsLoadingInProgress())
		{
			if (!Step->IsCompleteDelegateBound())
			{
				NxPrintVerbose(LogAsset, TEXT("[0x%X] Step %d - Still Loading (Listening)"), this, CurrentAsyncStep + 1);
				const bool bBound = Step->BindCompleteDelegate(FSimpleDelegate::CreateSP(this, &FNxLoadingState::TryCompleteAsyncLoading));
				ensureMsgf(bBound, TEXT("This is not intended to return false.  We're checking if it's loaded above, this should definitely return true."));
			}
			else
			{
				NxPrintVerbose(LogAsset, TEXT("[0x%X] Step %d - Still Loading (Waiting)"), this, CurrentAsyncStep + 1);
			}

			break;
		}
		else
		{
			NxPrintVerbose(LogAsset, TEXT("[0x%X] Step %d - Completed (Calling User)"), this, CurrentAsyncStep + 1);

			// Always advance the CurrentAsyncStep, before calling the user callback, it's possible they might
			// add new work, and try and start again, so we need to be ready for the next bit.
			CurrentAsyncStep++;

			Step->ExecuteUserCallback();
		}
	}
	
	// If we're done loading, and bHasStarted is still true (meaning this is the first time we're encountering a request to complete)
	// try and complete.  It's entirely possible that a user callback might append new work, which they immediately start, which
	// immediately tries to complete, which might create a case where we're now inside of TryCompleteAsyncLoading, which then
	// calls Start, which then calls TryCompleteAsyncLoading, so when we come back out of the stack, we need to avoid trying to
	// complete the async loading N+ times.
	if (IsLoadingComplete() && bHasStarted)
	{
		CompleteAsyncLoading();
	}
}

void FNxAsync::FNxLoadingState::CompleteAsyncLoading()
{
	NxPrintVerbose(LogAsset, TEXT("[0x%X] CompleteAsyncLoading"), this);

	// Mark that we've completed loading.
	if (bHasStarted)
	{
		bHasStarted = false;
		OwnerRef.OnFinishedLoading();
	}

	// It's unlikely but possible they started loading more stuff in the OnFinishedLoading callback,
	// so double check that we're still actually done.
	//
	// NOTE: We don't delete ourselves from memory in use.  Doing things like
	// pre-loading a bundle requires keeping the streaming handle alive.  So we're keeping
	// things alive.
	// 
	// We won't destroy the memory but we need to cleanup anything that may be hanging on to
	// captured scope, like completion handlers.
	if (IsLoadingComplete())
	{
		if (!bPreloadedBundles && !IsLoadingInProgressOrPending())
		{
			// If we're all done loading or pending loading, we should clean up the memory we're using.
			// go ahead and remove this loading state the owner mix-in allocated.
			RequestDestroyThisMemory();
			return;
		}
	}
}

//------------------------------------------------------------------------------

FNxAsync::FNxLoadingState::FNxAsyncStep::FNxAsyncStep(const FSimpleDelegate& InUserCallback)
	: UserCallback(InUserCallback)
{
}

FNxAsync::FNxLoadingState::FNxAsyncStep::FNxAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle)
	: UserCallback(InUserCallback)
	, StreamingHandle(InStreamingHandle)
{
}

FNxAsync::FNxLoadingState::FNxAsyncStep::FNxAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FNxAsyncCondition>& InCondition)
	: UserCallback(InUserCallback)
	, Condition(InCondition)
{
}

FNxAsync::FNxLoadingState::FNxAsyncStep::~FNxAsyncStep()
{

}

void FNxAsync::FNxLoadingState::FNxAsyncStep::ExecuteUserCallback()
{
	UserCallback.ExecuteIfBound();
	UserCallback.Unbind();
}

bool FNxAsync::FNxLoadingState::FNxAsyncStep::IsComplete() const
{
	if (StreamingHandle.IsValid())
	{
		return StreamingHandle->HasLoadCompleted();
	}
	else if (Condition.IsValid())
	{
		return Condition->IsComplete();
	}

	return true;
}

void FNxAsync::FNxLoadingState::FNxAsyncStep::Cancel()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->BindCompleteDelegate(FSimpleDelegate());
		StreamingHandle.Reset();
	}
	else if (Condition.IsValid())
	{
		Condition.Reset();
	}

	bIsCompletionDelegateBound = false;
}

bool FNxAsync::FNxLoadingState::FNxAsyncStep::BindCompleteDelegate(const FSimpleDelegate& NewDelegate)
{
	if (IsComplete())
	{
		// Too Late!
		return false;
	}

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->BindCompleteDelegate(NewDelegate);
	}
	else if (Condition)
	{
		Condition->BindCompleteDelegate(NewDelegate);
	}

	bIsCompletionDelegateBound = true;

	return true;
}

bool FNxAsync::FNxLoadingState::FNxAsyncStep::IsCompleteDelegateBound() const
{
	return bIsCompletionDelegateBound;
}

//------------------------------------------------------------------------------

FNxAsyncCondition::FNxAsyncCondition(const FOnAsyncConditionDelegate& Condition)
	: UserCondition(Condition)
{
}

FNxAsyncCondition::FNxAsyncCondition(TFunction<E_AsyncConditionResult()>&& Condition)
	: UserCondition(FOnAsyncConditionDelegate::CreateLambda([UserFunction = MoveTemp(Condition)]() mutable { return UserFunction(); }))
{
}

FNxAsyncCondition::~FNxAsyncCondition()
{
	FTSTicker::GetCoreTicker().RemoveTicker(RepeatHandle);
}

bool FNxAsyncCondition::IsComplete() const
{
	if (UserCondition.IsBound())
	{
		const E_AsyncConditionResult Result = UserCondition.Execute();
		return Result == E_AsyncConditionResult::Complete;
	}

	return true;
}

bool FNxAsyncCondition::BindCompleteDelegate(const FSimpleDelegate& NewDelegate)
{
	if (IsComplete())
	{
		// Already Complete
		return false;
	}

	CompletionDelegate = NewDelegate;

	if (!RepeatHandle.IsValid())
	{
		RepeatHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(this, &FNxAsyncCondition::TryToContinue), 0.16);
	}

	return true;
}

bool FNxAsyncCondition::TryToContinue(float)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncCondition_TryToContinue);

	NxPrintVerbose(LogAsset, TEXT("[0x%X] AsyncCondition::TryToContinue"), this);

	if (UserCondition.IsBound())
	{
		const E_AsyncConditionResult Result = UserCondition.Execute();

		switch (Result)
		{
		case E_AsyncConditionResult::TryAgain:
			return true;
		case E_AsyncConditionResult::Complete:
			RepeatHandle.Reset();
			UserCondition.Unbind();

			CompletionDelegate.ExecuteIfBound();
			CompletionDelegate.Unbind();
			break;
		}
	}

	return false;
}
