// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AZStoreSubsystem.generated.h"

class UAZStoreWidget;
class AAZPlayerController;
/**
 *
 */
UCLASS()
class ASHZERO_API UAZStoreSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void InitStoreWidget(TSubclassOf<UAZStoreWidget> WidgetClass);
	void ShowWidget();
	void CloseWidget();

	UPROPERTY()
	TObjectPtr<UAZStoreWidget> StoreWidget;

	bool BuyItem(AAZPlayerController* Requestor, int32 ItemID, int32 Quantity);

	void SellItem(AAZPlayerController* Requestor, int32 ItemID, int32 Quantity);
private:
	void RemoveWidget();
};
