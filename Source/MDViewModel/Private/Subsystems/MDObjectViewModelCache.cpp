#include "Subsystems/MDObjectViewModelCache.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "ViewModel/MDViewModelBase.h"

void FMDObjectViewModelCache::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddStableReferenceMap(CachedViewModels);
}

void FMDObjectViewModelCache::HandleObjectDestroyed()
{
	BroadcastShutdown();
}

UMDObjectViewModelCacheSystem* UMDObjectViewModelCacheSystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		return nullptr;
	}

	return UGameInstance::GetSubsystem<UMDObjectViewModelCacheSystem>(World->GetGameInstance());
}

bool UMDObjectViewModelCacheSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// Only create an instance if there is no override implementation defined elsewhere
	return ChildClasses.Num() == 0;
}

void UMDObjectViewModelCacheSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PostGarbageCollectConditionalBeginDestroy.AddUObject(this, &UMDObjectViewModelCacheSystem::CheckCacheForDestroyedObjects);
}

void UMDObjectViewModelCacheSystem::Deinitialize()
{
	FCoreUObjectDelegates::PostGarbageCollectConditionalBeginDestroy.RemoveAll(this);
	
	for (auto It = ObjectCacheMap.CreateIterator(); It; ++It)
	{
		It.Value().HandleObjectDestroyed();
	}
	
	Super::Deinitialize();
}

IMDViewModelCacheInterface* UMDObjectViewModelCacheSystem::ResolveCacheForObject(UObject* Object)
{
	UMDObjectViewModelCacheSystem* CacheSystem = UMDObjectViewModelCacheSystem::Get(Object);
	if (!IsValid(CacheSystem))
	{
		return nullptr;
	}

	FMDObjectViewModelCache& Cache = CacheSystem->ObjectCacheMap.FindOrAdd(Object);
	if (Cache.Object != Object)
	{
		Cache.Object = Object;
	}

	return &Cache;
}

IMDViewModelCacheInterface* UMDObjectViewModelCacheSystem::ResolveCacheForObject(const UObject* Object)
{
	UMDObjectViewModelCacheSystem* CacheSystem = UMDObjectViewModelCacheSystem::Get(Object);
	if (!IsValid(CacheSystem))
	{
		return nullptr;
	}

	return CacheSystem->ObjectCacheMap.Find(Object);
}

void UMDObjectViewModelCacheSystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	if (UMDObjectViewModelCacheSystem* CacheSystem = Cast<UMDObjectViewModelCacheSystem>(InThis))
	{
		for (auto It = CacheSystem->ObjectCacheMap.CreateIterator(); It; ++It)
		{
			It.Value().AddReferencedObjects(Collector);
		}
	}
}

void UMDObjectViewModelCacheSystem::CheckCacheForDestroyedObjects()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	for (auto It = ObjectCacheMap.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.Value().HandleObjectDestroyed();
			It.RemoveCurrent();
		}
	}
}
