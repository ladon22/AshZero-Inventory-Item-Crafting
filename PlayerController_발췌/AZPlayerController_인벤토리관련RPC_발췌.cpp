bool AAZPlayerController::IsInventoryComponentAccessible(UAZInventoryComponent* Comp) const
{
	if (!Comp) return false;

	if (Comp == InventoryComp || Comp == StashComp)
		return true;

	if (CurrentOpenChest && Comp == CurrentOpenChest->LootBoxComponent)
		return true;

	if (EquipmentComp)
	{
		if (UAZBagItem* EquippedBag = Cast<UAZBagItem>(EquipmentComp->Backpack))
		{
			if (EquippedBag->InventoryComponent == Comp)
				return true;
		}
	}

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
	if (UWorld* World = GetWorld())
	{
		if (UAZStoreSubsystem* StoreSubsystem = World->GetSubsystem<UAZStoreSubsystem>())
		{
			bool bSuccess = StoreSubsystem->BuyItem(this, ItemID, Quantity);
		}
	}
}

void AAZPlayerController::SellStoreItem_Server_Implementation(int32 ItemID, int32 Quantity)
{
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
	if (!CraftingComponent)
	{
		return;
	}

	if (CraftingComponent->GetLinkedStashComponent() != StashComp)
	{
		return;
	}

	bool bStarted = CraftingComponent->TryStartCrafting(JobID);
}

void AAZPlayerController::CollectCraftedItem_Server_Implementation(UAZCraftingComponent* CraftingComponent, int32 JobID)
{

	if (!CraftingComponent) return;
	if (CraftingComponent->GetLinkedStashComponent() != StashComp)
	{
		return;
	}

	CraftingComponent->CollectCraftedItem(JobID);
}
