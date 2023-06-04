﻿#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Modules/ModuleManager.h"

class UBlueprint;
class UMDViewModelBase;
class UMDViewModelBlueprintCompilerExtension;
class FWorkflowAllowedTabSet;
class FWidgetBlueprintApplicationMode;

class FMDViewModelEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void HandleRegisterBlueprintEditorTab(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories);

	void HandleActivateMode(FWidgetBlueprintApplicationMode& InDesignerMode);

	void HandleDeactivateMode(FWidgetBlueprintApplicationMode& InDesignerMode);

	UMDViewModelBlueprintCompilerExtension* CompilerExtension = nullptr;
};
