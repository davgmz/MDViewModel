#pragma once

#include "CoreMinimal.h"
#include "BlueprintCompilerExtension.h"
#include "MDViewModelBlueprintCompilerExtension.generated.h"

/**
 * Blueprint Compiler Extension to inject MDViewModelClassExtensions when compiling widgets with super classes that have view model assignments
 */
UCLASS()
class MDVIEWMODELEDITOR_API UMDViewModelBlueprintCompilerExtension : public UBlueprintCompilerExtension
{
	GENERATED_BODY()

public:
	virtual void ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data) override;
};
