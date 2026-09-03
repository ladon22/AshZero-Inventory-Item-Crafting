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

	// Requestor: 이 요청을 보낸 플레이어. 서버 RPC(AAZPlayerController::BuyStoreItem_Server 등)를
	// 통해서만 호출되어야 하며, 반드시 그 RPC를 수신한 PlayerController(this)를 그대로 넘겨야 합니다.
	// World->GetFirstPlayerController()로 임의 추정하지 않습니다.
	bool BuyItem(AAZPlayerController* Requestor, int32 ItemID, int32 Quantity);

	void SellItem(AAZPlayerController* Requestor, int32 ItemID, int32 Quantity);
private:
	void RemoveWidget();
};
