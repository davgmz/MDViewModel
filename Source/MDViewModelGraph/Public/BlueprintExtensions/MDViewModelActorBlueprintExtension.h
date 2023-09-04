#pragma once

#include "Blueprint/BlueprintExtension.h"
#include "MDViewModelAssignableInterface.h"
#include "Util/MDViewModelEditorAssignment.h"
#include "MDViewModelActorBlueprintExtension.generated.h"

/**
 * Editor-only class that holds design-time assigned view models for actors
 */
UCLASS()
class MDVIEWMODELGRAPH_API UMDViewModelActorBlueprintExtension : public UBlueprintExtension, public IMDViewModelAssignableInterface
{
	GENERATED_BODY()

public:
	virtual const TArray<FMDViewModelEditorAssignment>& GetAssignments() const override { return Assignments; }
	virtual TArray<FMDViewModelEditorAssignment>& GetAssignments() override { return Assignments; }

protected:
	UPROPERTY()
	TArray<FMDViewModelEditorAssignment> Assignments;
	
private:
	virtual void HandleGenerateFunctionGraphs(FKismetCompilerContext* CompilerContext) override;
	
	virtual void SearchParentAssignments(TMap<FMDViewModelAssignment, FMDViewModelAssignmentData>& OutViewModelAssignments, TSubclassOf<UMDViewModelBase> ViewModelClass = nullptr, const FGameplayTag& ProviderTag = FGameplayTag::EmptyTag, const FName& ViewModelName = NAME_None) const override;
};
