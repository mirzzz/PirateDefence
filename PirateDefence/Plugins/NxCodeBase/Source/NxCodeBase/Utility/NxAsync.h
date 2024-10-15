// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "Engine/AssetManager.h"
#include "Async/Future.h"
#include "Containers/Ticker.h"

class FNxAsyncCondition;
class UPrimaryDataAsset;

DECLARE_DELEGATE_OneParam(FStreamableHandleDelegate, TSharedPtr<FStreamableHandle>)

//TODO I think we need to introduce a retention policy, preloads automatically stay in memory until canceled
//     but what if you want to preload individual items just using the AsyncLoad functions?  I don't want to
//     introduce individual policies per call, or introduce a whole set of preload vs asyncloads, so would
//     would rather have a retention policy.  Should it be a member and actually create real memory when
//     you inherit from VxAsync, or should it be a template argument?
//enum class EAsyncMixinRetentionPolicy : uint8
//{
//	Default,
//	KeepResidentUntilComplete,
//	KeepResidentUntilCancel
//};

/**
  FNxAsync 를 사용하면 비동기 로딩 요청을 더 쉽게 관리하여 선형 요청 처리를 보장하고, 코드 작성이 훨씬 쉽습니다. 
	- 로딩과 같은 비동기 작업을 관리하기 위한 C++ 유틸리티 클래스

	사용 패턴은 다음과 같습니다.

		FNxAsync 에서 상속합니다. UObject 이더라도 FNxAsync 에서 상속할 수도 있습니다.
		상속 후에, 다음과 같이 비동기 로드를 만들 수 있습니다.

		일부 객체는 목록에서처럼 재사용되므로 보류 중인 완료되지 않은 항목을 취소하는 것이 중요합니다.
		CancelAsyncLoading();			
		AsyncLoad(ItemOne, CallbackOne);
		AsyncLoad(ItemTwo, CallbackTwo);
		StartAsyncLoading();

		VxAsync 이점 중 하나인 'this' 범위를 안전하게 포함할 수도 있습니다. 
		어떤 콜백도 호스트 VxAsync 파생 개체의 범위를 벗어나지 않는다는 것입니다.
		e.g.
		AsyncLoad(SomeSoftObjectPtr, [this, ...]() { ... });

		- 작업전 기존것들을 먼저 취소 시킨다.
		- 위젯에 새로운 작업을 진행할 경우, 비동기 로드를 요청한 순서대로 ItemOne 및 ItemTwo를 로드하고 콜백을 호출합니다.
		  요청했을 때 ItemOne 또는 ItemTwo가 이미 로드된 경우에도 마찬가지입니다.
		- 모든 비동기 로드 요청이 완료되면 OnFinishedLoading이 호출됩니다.
		- StartAsyncLoading() 호출 을 잊어버린 경우, 다음 프레임에서 호출 하지만 설정이 완료되면 호출해야 함을 기억해야 합니다.
		  그렇지 않은 경우, 로드시 깜박임이 발생할수 있습니다.
 
	NOTE: FNxAsync는 [this]를 캡처된 입력으로 람다에 전달하는 것도 안전합니다. 
          소유자 클래스가 파괴되거나 모든 것을 취소하는 경우 모든 후크 해제를 처리하기 때문입니다.

	NOTE: FNxAsync는 클래스에 추가 메모리를 추가하지 않습니다. 
          현재 비동기 로드를 처리하는 여러 클래스는 내부적으로 TSharedPtr<FStreamableHandle> 멤버를 할당하고 
          SoftObjectPaths 임시 상태를 유지하는 경향이 있습니다.
          FNxAsync는 모든 비동기 요청 메모리가 일시적으로 희소하게 저장되도록 정적 TMap을 사용하여 이 모든 작업을 내부적으로 수행합니다.
 
	NOTE: 디버깅 및 진행 상황을 이해하려면 명령줄에 -LogCmds="LogNxAsync Verbose"를 추가해야 합니다
 */

class NXCODEBASE_API FNxAsync : public FNoncopyable
{
protected:
	FNxAsync();

public:
	virtual ~FNxAsync();

protected:
	/** Called when loading starts. */
	virtual void OnStartedLoading() { }
	/** Called when all loading has finished. */
	virtual void OnFinishedLoading() { }

protected:
	/** Async load a TSoftClassPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** Async load a TSoftClassPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void(TSubclassOf<T>)>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(),
			FSimpleDelegate::CreateLambda([SoftClass, UserCallback = MoveTemp(Callback)]() mutable {
				UserCallback(SoftClass.Get());
			})
		);
	}

	/** Async load a TSoftClassPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), Callback);
	}

	/** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void(T*)>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(),
			FSimpleDelegate::CreateLambda([SoftObject, UserCallback = MoveTemp(Callback)]() mutable {
				UserCallback(SoftObject.Get());
			})
		);
	}

	/** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), Callback);
	}

	/** Async load a FSoftObjectPath, call the Callback when complete. */
	void AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** Async load an array of FSoftObjectPath, call the Callback when complete. */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObjectPaths, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** Async load an array of FSoftObjectPath, call the Callback when complete. */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** Given an array of primary assets, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
	template<typename T = UPrimaryDataAsset>
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<T*>& Assets, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		TArray<FPrimaryAssetId> PrimaryAssetIds;
		for (const T* Item : Assets)
		{
			PrimaryAssetIds.Add(Item);
		}

		AsyncPreloadPrimaryAssetsAndBundles(PrimaryAssetIds, LoadBundles, Callback);
	}

	/** Given an array of primary asset ids, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, TFunction<void()>&& Callback)
	{
		AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** Given an array of primary asset ids, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 앞으로 나아가기 전에 반드시 참이어야 하는 미래 조건을 추가하십시오. */
	void AsyncCondition(TSharedRef<FNxAsyncCondition> Condition, const FSimpleDelegate& Callback = FSimpleDelegate());

	/**
	 * Rather than load anything, this callback is just inserted into the callback sequence so that when async loading 
	 * completes this event will be called at the same point in the sequence.  Super useful if you don't want a step to be
	 * tied to a particular asset in case some of the assets are optional.
	 */
	void AsyncEvent(TFunction<void()>&& Callback)
	{
		AsyncEvent(FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/**
	 * 아무 것도 로드하지 않고 이 콜백은 콜백 시퀀스에 삽입되어 비동기 로드가 완료되면 시퀀스의 동일한 지점에서 이 이벤트가 호출됩니다. 
	 * 자산 중 일부가 선택 사항인 경우 특정 자산에 단계를 연결하지 않으려는 경우 매우 유용합니다.
	 */
	void AsyncEvent(const FSimpleDelegate& Callback);

	/** 모든 비동기 로딩 요청을 플러시합니다. */
	void StartAsyncLoading();

	/** 보류 중인 모든 비동기 로드를 취소합니다. */
	void CancelAsyncLoading();

	/** 비동기 로딩이 현재 진행 중입니까? */
	bool IsAsyncLoadingInProgress() const;

private:
	/**
	 * FNxLoadingState는 FNxAsync 자체가 메모리를 보유하지 않도록 큰 맵에서 FNxAsync에 실제로 할당된 것이며, 
	 * 필요할 경우에만 FNxLoadingState를 동적으로 생성하고 필요하지 않을 때 이를 파괴합니다.
	 */
	class FNxLoadingState : public TSharedFromThis<FNxLoadingState>
	{
	public:
		FNxLoadingState(FNxAsync& InOwner);
		virtual ~FNxLoadingState();

		/** Starts the async sequence. */
		void Start();

		/** Cancels the async sequence. */
		void CancelAndDestroy();

		void AsyncLoad(FSoftObjectPath SoftObject, const FSimpleDelegate& DelegateToCall);
		void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall);
		void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& PrimaryAssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall);
		void AsyncCondition(TSharedRef<FNxAsyncCondition> Condition, const FSimpleDelegate& Callback);
		void AsyncEvent(const FSimpleDelegate& Callback);

		bool IsLoadingComplete() const { return !IsLoadingInProgress(); }
		bool IsLoadingInProgress() const;
		bool IsLoadingInProgressOrPending() const;
		bool IsPendingDestroy() const;

	private:
		void CancelOnly(bool bDestroying);
		void CancelStartTimer();
		void TryScheduleStart();
		void TryCompleteAsyncLoading();
		void CompleteAsyncLoading();

	private:
		void RequestDestroyThisMemory();
		void CancelDestroyThisMemory(bool bDestroying);

		// 로드 상태는 소유자, 소유하는 Async 객체를 다시 호출하려면 필요합니다
		FNxAsync& OwnerRef;

		// 번들을 미리 로드해야 할지 여부.
		// 번들을 미리 로드하지 않은 경우(스트리밍 핸들을 유지해야 하며 그렇지 않으면 파괴될 것입니다)
		// 모든 로드가 완료되면 FLoadingState를 안전하게 파괴할 수 있습니다.
		bool bPreloadedBundles = false;

		class FNxAsyncStep
		{
		public:
			FNxAsyncStep(const FSimpleDelegate& InUserCallback);
			FNxAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle);
			FNxAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FNxAsyncCondition>& InCondition);

			~FNxAsyncStep();

			void ExecuteUserCallback();

			bool IsLoadingInProgress() const
			{
				return !IsComplete();
			}

			bool IsComplete() const;
			void Cancel();

			bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);
			bool IsCompleteDelegateBound() const;

		private:
			FSimpleDelegate UserCallback;
			bool bIsCompletionDelegateBound = false;

			// Possible Async 'thing'
			TSharedPtr<FStreamableHandle> StreamingHandle;
			TSharedPtr<FNxAsyncCondition> Condition;
		};

		bool bHasStarted = false;

		int32 CurrentAsyncStep = 0;
		TArray<TUniquePtr<FNxAsyncStep>> AsyncSteps;
		TArray<TUniquePtr<FNxAsyncStep>> AsyncStepsPendingDestruction;

		FTSTicker::FDelegateHandle StartTimerDelegate;
		FTSTicker::FDelegateHandle DestroyMemoryDelegate;
	};

	const FNxLoadingState& GetLoadingStateConst() const;
	
	FNxLoadingState& GetLoadingState();

	bool HasLoadingState() const;

	bool IsLoadingInProgressOrPending() const;

private:
	static TMap<FNxAsync*, TSharedRef<FNxLoadingState>> Loading;
};

/**
 * 혼합 비동기 로딩이 아닌, 고유한 비동기 종속성이 필요한 경우, FNxAsyncScope는 종속성 관리를 지원한다.
 * 
 * 독립 실행형 Async 종속성 핸들러로, 
 * FNxAsync를 클래스와 결합하는 것처럼 여러 로드 작업을 시작하고 항상 적절한 순서로 처리할 수 있습니다
 */
class NXCODEBASE_API FNxAsyncScope : public FNxAsync
{
public:
	using FNxAsync::AsyncLoad;

	using FNxAsync::AsyncPreloadPrimaryAssetsAndBundles;

	using FNxAsync::AsyncCondition;

	using FNxAsync::AsyncEvent;

	using FNxAsync::CancelAsyncLoading;

	using FNxAsync::StartAsyncLoading;

	using FNxAsync::IsAsyncLoadingInProgress;
};

//------------------------------------------------------------------------------
enum class E_AsyncConditionResult : uint8
{
	TryAgain,
	Complete
};

DECLARE_DELEGATE_RetVal(E_AsyncConditionResult, FOnAsyncConditionDelegate);

/**
 * 비동기 조건을 사용하면 일부 조건이 충족될 때까지 비동기 로드를 중지하는 사용자 조작을 지원한다.
 */
class FNxAsyncCondition : public TSharedFromThis<FNxAsyncCondition>
{
public:
	FNxAsyncCondition(const FOnAsyncConditionDelegate& Condition);
	FNxAsyncCondition(TFunction<E_AsyncConditionResult()>&& Condition);
	virtual ~FNxAsyncCondition();

protected:
	bool IsComplete() const;
	bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);

private:
	bool TryToContinue(float DeltaTime);

	FTSTicker::FDelegateHandle	RepeatHandle;
	FOnAsyncConditionDelegate	UserCondition;
	FSimpleDelegate				CompletionDelegate;

	friend FNxAsync;
};
