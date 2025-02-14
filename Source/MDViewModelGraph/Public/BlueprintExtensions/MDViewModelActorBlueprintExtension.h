#pragma once

#include "Blueprint/BlueprintExtension.h"
#include "MDViewModelAssignableInterface.h"
#include "Util/MDViewModelEditorAssignment.h"
#include "MDViewModelActorBlueprintExtension.generated.h"

/**
 * Editor-only class that holds design-time assigned view models for actors
 * Assignment compilation happens in UMDViewModelBlueprintCompilerExtension::HandleActorBlueprintPreCompile
 */
UCLASS()
class MDVIEWMODELGRAPH_API UMDViewModelActorBlueprintExtension : public UBlueprintExtension, public IMDViewModelAssignableInterface
{
	GENERATED_BODY()

public:
	virtual TArray<FMDViewModelEditorAssignment>& GetAssignments() override { return Assignments; }

protected:
	UPROPERTY()
	TArray<FMDViewModelEditorAssignment> Assignments;
	
};
