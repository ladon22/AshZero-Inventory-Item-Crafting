	UFUNCTION(Server, Reliable)
	void TryStackItem_Server(UAZInventoryComponent* SourceComponent, UAZInventoryComponent* DestComponent, UAZItemBase* SourceItem, UAZItemBase* DestItem, UAZItemWidget* SourceWidget, UAZItemWidget* DestWidget);

	bool IsInventoryComponentAccessible(UAZInventoryComponent* Comp) const;

	UFUNCTION(Server, Reliable)
	void MoveItem_Server(UAZInventoryComponent* SourceComp, UAZInventoryComponent* DestComp, UAZItemBase* Item, int32 TopLeftIndex, bool bIsRotated);

	UFUNCTION(Server, Reliable)
	void BuyStoreItem_Server(int32 ItemID, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void SellStoreItem_Server(int32 ItemID, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void StartCrafting_Server(UAZCraftingComponent* CraftingComponent, int32 JobID);

	UFUNCTION(Server, Reliable)
	void CollectCraftedItem_Server(UAZCraftingComponent* CraftingComponent, int32 JobID);
