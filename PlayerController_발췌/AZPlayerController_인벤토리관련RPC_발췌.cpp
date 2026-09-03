// 이 파일은 팀 공용 파일인 System/Player/AZPlayerController.cpp 전체가 아니라,
// 본인이 담당한 인벤토리/아이템/제작/상점 관련 Server RPC 구현부만 발췌한 것입니다.
// 소유권 검증(IsInventoryComponentAccessible, GetLinkedStashComponent 비교)을 통해
// 클라이언트가 임의의 컴포넌트/제작대 포인터를 넘겨 다른 플레이어의 데이터를 조작하는 것을 서버 단에서 차단합니다.
// (원본 경로: Source/AshZero/System/Player/AZPlayerController.cpp)

bool AAZPlayerController::IsInventoryComponentAccessible(UAZInventoryComponent* Comp) const
{
	if (!Comp) return false;

	// 본인 인벤토리 / 창고
	if (Comp == InventoryComp || Comp == StashComp)
		return true;

	// 현재 서버가 이 플레이어에게 열어준(RequestOpenLootBox_Server로 등록된) 상자
	if (CurrentOpenChest && Comp == CurrentOpenChest->LootBoxComponent)
		return true;

	// 장착 중인 가방의 내부 인벤토리
	if (EquipmentComp)
	{
		if (UAZBagItem* EquippedBag = Cast<UAZBagItem>(EquipmentComp->Backpack))
		{
			if (EquippedBag->InventoryComponent == Comp)
				return true;
		}
	}

	// 내 인벤토리/창고 안에 들어있는(장착은 안 했지만 소지 중인) 가방의 내부 인벤토리
	auto ContainsBagWithInventory = [Comp](UAZInventoryComponent* Container) -> bool
	{
		if (!Container) return false;
		for (UAZItemBase* Entry : Container->Items)
		{
			if (UAZBagItem* Bag = Cast<UAZBagItem>(Entry))
			{
				if (Bag->InventoryComponent == Comp)
					return true;
			}
		}
		return false;
	};

	if (ContainsBagWithInventory(InventoryComp) || ContainsBagWithInventory(StashComp))
		return true;

	PRINT_LOG(TEXT("[AntiCheat] %s: %s가 접근 불가능한 인벤토리 컴포넌트(%s)를 요청함 - 거부"),
		*GetName(), *GetNameSafe(GetPawn()), *GetNameSafe(Comp));
	return false;
}

void AAZPlayerController::TryStackItem_Server_Implementation(UAZInventoryComponent* SourceComponent, UAZInventoryComponent* DestComponent, UAZItemBase* SourceItem, UAZItemBase* DestItem, UAZItemWidget* SourceWidget, UAZItemWidget* DestWidget)
{
	if (!IsInventoryComponentAccessible(SourceComponent) || !IsInventoryComponentAccessible(DestComponent))
		return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI == nullptr)
		return;
	UAZDataManagerSubsystem* DataManger = GI->GetSubsystem<UAZDataManagerSubsystem>();
	if (DataManger == nullptr)
		return;
	if (DestItem == nullptr)
		return;
	if (SourceItem == nullptr)
		return;

	if (SourceItem == DestItem)
	{
		SetItemDrag_Server(DestItem, false);
		return;
	}
	if (SourceItem->GetItemID() == DestItem->GetItemID())
	{
		int32 SourceItemStackcount = SourceItem->GetStackCount();
		int32 DestItemStackCount = DestItem->GetStackCount();
		int32 SumCount = SourceItemStackcount + DestItemStackCount;
		int32 MaxStackCount = DataManger->GetMaxStackSizeByID(DestItem->GetItemID());
		if (SumCount > MaxStackCount)	// 최대 스택보다 많다면
		{
			SourceComponent->SetItemStack_Multicast(SourceItem, SumCount - MaxStackCount);
			DestComponent->SetItemStack_Multicast(DestItem, MaxStackCount);
		}
		else
		{
			SourceComponent->RemoveItem_Server(SourceItem);
			DestComponent->SetItemStack_Multicast(DestItem, SumCount);
			SourceComponent->OnRep_Items();
		}
	}
	SetItemDrag_Server(DestItem, false);
}

void AAZPlayerController::MoveItem_Server_Implementation(UAZInventoryComponent* SourceComp, UAZInventoryComponent* DestComp, UAZItemBase* Item, int32 TopLeftIndex, bool bIsRotated)
{
	if (!IsInventoryComponentAccessible(SourceComp) || !IsInventoryComponentAccessible(DestComp))
		return;

	if (SourceComp && DestComp && Item)
	{
		Item->SetIsDragged(false);
		SourceComp->RemoveItem(Item);
		Item->SetIsRotated(bIsRotated);
		DestComp->AddItemAt(Item, TopLeftIndex);

		SourceComp->OnRep_Items();
		DestComp->OnRep_Items();
	}
}

void AAZPlayerController::BuyStoreItem_Server_Implementation(int32 ItemID, int32 Quantity)
{
	PRINT_LOG(TEXT("[Store] %s가 서버에 구매 요청 도착: ItemID=%d, Quantity=%d"), *GetNameSafe(GetPawn()), ItemID, Quantity);

	if (UWorld* World = GetWorld())
	{
		if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
		{
			// this(서버에서 이 RPC를 수신한, 연결이 확실한 PlayerController)를 그대로 넘겨서
			// 재화/아이템 변경이 항상 실제 요청자 본인에게만 적용되도록 합니다.
			bool bSuccess = StoreSubsystem->BuyItem(this, ItemID, Quantity);
			PRINT_LOG(TEXT("[Store] 구매 처리 결과: %s"), bSuccess ? TEXT("성공") : TEXT("실패"));
		}
	}
}

void AAZPlayerController::SellStoreItem_Server_Implementation(int32 ItemID, int32 Quantity)
{
	PRINT_LOG(TEXT("[Store] %s가 서버에 판매 요청 도착: ItemID=%d, Quantity=%d"), *GetNameSafe(GetPawn()), ItemID, Quantity);

	if (UWorld* World = GetWorld())
	{
		if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
		{
			StoreSubsystem->SellItem(this, ItemID, Quantity);
		}
	}
}

void AAZPlayerController::StartCrafting_Server_Implementation(UAZCraftingComponent* CraftingComponent, int32 JobID)
{
	PRINT_LOG(TEXT("[Crafting] %s가 서버에 제작 시작 요청 도착: CraftingComponent=%s, JobID=%d"),
		*GetNameSafe(GetPawn()), *GetNameSafe(CraftingComponent), JobID);

	if (!CraftingComponent)
	{
		PRINT_LOG(TEXT("[Crafting] CraftingComponent가 서버에서 nullptr로 수신됨 - 리플리케이션 설정을 확인하세요"));
		return;
	}

	// 이 제작대가 실제로 내 창고(StashComp)에 연결된 제작대인지 확인합니다. 이 검사가 없으면
	// 클라이언트가 다른 플레이어의 CraftingComponent 포인터를 넘겨 그 플레이어의 창고 재료를
	// 대신 소모시킬 수 있습니다.
	if (CraftingComponent->GetLinkedStashComponent() != StashComp)
	{
		PRINT_LOG(TEXT("[AntiCheat] %s가 본인 소유가 아닌 제작대에 대해 제작을 요청함 - 거부"), *GetNameSafe(GetPawn()));
		return;
	}

	bool bStarted = CraftingComponent->TryStartCrafting(JobID);
	PRINT_LOG(TEXT("[Crafting] 제작 시작 결과: %s"), bStarted ? TEXT("성공") : TEXT("실패(재료 부족 등)"));
}

void AAZPlayerController::CollectCraftedItem_Server_Implementation(UAZCraftingComponent* CraftingComponent, int32 JobID)
{
	PRINT_LOG(TEXT("[Crafting] %s가 서버에 제작물 수령 요청 도착: CraftingComponent=%s, JobID=%d"),
		*GetNameSafe(GetPawn()), *GetNameSafe(CraftingComponent), JobID);

	if (!CraftingComponent) return;
	if (CraftingComponent->GetLinkedStashComponent() != StashComp)
	{
		PRINT_LOG(TEXT("[AntiCheat] %s가 본인 소유가 아닌 제작대에서 수령을 요청함 - 거부"), *GetNameSafe(GetPawn()));
		return;
	}

	CraftingComponent->CollectCraftedItem(JobID);
}
