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
  FNxAsync �� ����ϸ� �񵿱� �ε� ��û�� �� ���� �����Ͽ� ���� ��û ó���� �����ϰ�, �ڵ� �ۼ��� �ξ� �����ϴ�. 
	- �ε��� ���� �񵿱� �۾��� �����ϱ� ���� C++ ��ƿ��Ƽ Ŭ����

	��� ������ ������ �����ϴ�.

		FNxAsync ���� ����մϴ�. UObject �̴��� FNxAsync ���� ����� ���� �ֽ��ϴ�.
		��� �Ŀ�, ������ ���� �񵿱� �ε带 ���� �� �ֽ��ϴ�.

		�Ϻ� ��ü�� ��Ͽ���ó�� ����ǹǷ� ���� ���� �Ϸ���� ���� �׸��� ����ϴ� ���� �߿��մϴ�.
		CancelAsyncLoading();			
		AsyncLoad(ItemOne, CallbackOne);
		AsyncLoad(ItemTwo, CallbackTwo);
		StartAsyncLoading();

		VxAsync ���� �� �ϳ��� 'this' ������ �����ϰ� ������ ���� �ֽ��ϴ�. 
		� �ݹ鵵 ȣ��Ʈ VxAsync �Ļ� ��ü�� ������ ����� �ʴ´ٴ� ���Դϴ�.
		e.g.
		AsyncLoad(SomeSoftObjectPtr, [this, ...]() { ... });

		- �۾��� �����͵��� ���� ��� ��Ų��.
		- ������ ���ο� �۾��� ������ ���, �񵿱� �ε带 ��û�� ������� ItemOne �� ItemTwo�� �ε��ϰ� �ݹ��� ȣ���մϴ�.
		  ��û���� �� ItemOne �Ǵ� ItemTwo�� �̹� �ε�� ��쿡�� ���������Դϴ�.
		- ��� �񵿱� �ε� ��û�� �Ϸ�Ǹ� OnFinishedLoading�� ȣ��˴ϴ�.
		- StartAsyncLoading() ȣ�� �� �ؾ���� ���, ���� �����ӿ��� ȣ�� ������ ������ �Ϸ�Ǹ� ȣ���ؾ� ���� ����ؾ� �մϴ�.
		  �׷��� ���� ���, �ε�� �������� �߻��Ҽ� �ֽ��ϴ�.
 
	NOTE: FNxAsync�� [this]�� ĸó�� �Է����� ���ٿ� �����ϴ� �͵� �����մϴ�. 
          ������ Ŭ������ �ı��ǰų� ��� ���� ����ϴ� ��� ��� ��ũ ������ ó���ϱ� �����Դϴ�.

	NOTE: FNxAsync�� Ŭ������ �߰� �޸𸮸� �߰����� �ʽ��ϴ�. 
          ���� �񵿱� �ε带 ó���ϴ� ���� Ŭ������ ���������� TSharedPtr<FStreamableHandle> ����� �Ҵ��ϰ� 
          SoftObjectPaths �ӽ� ���¸� �����ϴ� ������ �ֽ��ϴ�.
          FNxAsync�� ��� �񵿱� ��û �޸𸮰� �Ͻ������� ����ϰ� ����ǵ��� ���� TMap�� ����Ͽ� �� ��� �۾��� ���������� �����մϴ�.
 
	NOTE: ����� �� ���� ��Ȳ�� �����Ϸ��� ����ٿ� -LogCmds="LogNxAsync Verbose"�� �߰��ؾ� �մϴ�
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

	/** ������ ���ư��� ���� �ݵ�� ���̾�� �ϴ� �̷� ������ �߰��Ͻʽÿ�. */
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
	 * �ƹ� �͵� �ε����� �ʰ� �� �ݹ��� �ݹ� �������� ���ԵǾ� �񵿱� �ε尡 �Ϸ�Ǹ� �������� ������ �������� �� �̺�Ʈ�� ȣ��˴ϴ�. 
	 * �ڻ� �� �Ϻΰ� ���� ������ ��� Ư�� �ڻ꿡 �ܰ踦 �������� �������� ��� �ſ� �����մϴ�.
	 */
	void AsyncEvent(const FSimpleDelegate& Callback);

	/** ��� �񵿱� �ε� ��û�� �÷����մϴ�. */
	void StartAsyncLoading();

	/** ���� ���� ��� �񵿱� �ε带 ����մϴ�. */
	void CancelAsyncLoading();

	/** �񵿱� �ε��� ���� ���� ���Դϱ�? */
	bool IsAsyncLoadingInProgress() const;

private:
	/**
	 * FNxLoadingState�� FNxAsync ��ü�� �޸𸮸� �������� �ʵ��� ū �ʿ��� FNxAsync�� ������ �Ҵ�� ���̸�, 
	 * �ʿ��� ��쿡�� FNxLoadingState�� �������� �����ϰ� �ʿ����� ���� �� �̸� �ı��մϴ�.
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

		// �ε� ���´� ������, �����ϴ� Async ��ü�� �ٽ� ȣ���Ϸ��� �ʿ��մϴ�
		FNxAsync& OwnerRef;

		// ������ �̸� �ε��ؾ� ���� ����.
		// ������ �̸� �ε����� ���� ���(��Ʈ���� �ڵ��� �����ؾ� �ϸ� �׷��� ������ �ı��� ���Դϴ�)
		// ��� �ε尡 �Ϸ�Ǹ� FLoadingState�� �����ϰ� �ı��� �� �ֽ��ϴ�.
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
 * ȥ�� �񵿱� �ε��� �ƴ�, ������ �񵿱� ���Ӽ��� �ʿ��� ���, FNxAsyncScope�� ���Ӽ� ������ �����Ѵ�.
 * 
 * ���� ������ Async ���Ӽ� �ڵ鷯��, 
 * FNxAsync�� Ŭ������ �����ϴ� ��ó�� ���� �ε� �۾��� �����ϰ� �׻� ������ ������ ó���� �� �ֽ��ϴ�
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
 * �񵿱� ������ ����ϸ� �Ϻ� ������ ������ ������ �񵿱� �ε带 �����ϴ� ����� ������ �����Ѵ�.
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
