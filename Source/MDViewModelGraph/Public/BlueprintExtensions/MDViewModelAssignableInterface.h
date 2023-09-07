#pragma once

#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "MDViewModelAssignableInterface.generated.h"

class UMDViewModelBase;
struct FMDViewModelAssignmentData;
struct FMDViewModelAssignment;
struct FMDViewModelEditorAssignment;

UINTERFACE()
class UMDViewModelAssignableInterface : public UInterface
{
	GENERATED_BODY()
};

// Interface for an object that can be assigned view models
class MDVIEWMODELGRAPH_API IMDViewModelAssignableInterface
{
	GENERATED_BODY()

public:
	virtual const TArray<FMDViewModelEditorAssignment>& GetAssignments() const = 0;
	virtual TArray<FMDViewModelEditorAssignment>& GetAssignments() = 0;
	
	virtual UBlueprint* GetBlueprint() const;

	void ModifyObject();
	
	void GetAllAssignments(TMap<FMDViewModelAssignment, FMDViewModelAssignmentData>& OutViewModelAssignments) const;

	void AddAssignment(FMDViewModelEditorAssignment&& Assignment);
	void UpdateAssignment(const FMDViewModelEditorAssignment& Assignment, FMDViewModelEditorAssignment&& UpdatedAssignment);
	void RemoveAssignment(const FMDViewModelEditorAssignment& Assignment);

	bool DoesContainViewModelAssignment(TSubclassOf<UMDViewModelBase> ViewModelClass = nullptr, const FGameplayTag& ProviderTag = FGameplayTag::EmptyTag, const FName& ViewModelName = NAME_None) const;
	bool HasAssignments() const;

	FSimpleMulticastDelegate OnAssignmentsChanged;

	DECLARE_MULTICAST_DELEGATE_FourParams(FOnViewModelAssignmentChanged, const FName& /*OldName*/, const FName& /*NewName*/, TSubclassOf<UMDViewModelBase> /*OldClass*/, TSubclassOf<UMDViewModelBase> /*NewClass*/);
	FOnViewModelAssignmentChanged OnAssignmentChanged;

protected:
	void GetParentAssignments(TMap<FMDViewModelAssignment, FMDViewModelAssignmentData>& OutViewModelAssignments) const;
	void SearchParentAssignments(TMap<FMDViewModelAssignment, FMDViewModelAssignmentData>& OutViewModelAssignments, TSubclassOf<UMDViewModelBase> ViewModelClass = nullptr, const FGameplayTag& ProviderTag = FGameplayTag::EmptyTag, const FName& ViewModelName = NAME_None) const;
};
