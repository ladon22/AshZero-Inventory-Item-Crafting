// 이 파일은 팀 공용 파일인 System/Player/AZPlayerController.h 전체가 아니라,
// 본인이 담당한 인벤토리/아이템/제작/상점 관련 Server RPC 선언부만 발췌한 것입니다.
// 원본 AZPlayerController에는 이동, 카메라, 상호작용 등 다른 팀원이 작성한 로직도 포함되어 있습니다.
// (원본 경로: Source/AshZero/System/Player/AZPlayerController.h, 135~167번째 줄 부근)

	UFUNCTION(Server, Reliable)
	void TryStackItem_Server(UAZInventoryComponent* SourceComponent, UAZInventoryComponent* DestComponent, UAZItemBase* SourceItem, UAZItemBase* DestItem, UAZItemWidget* SourceWidget, UAZItemWidget* DestWidget);

	// SourceComp/DestComp/InventoryComponent 등으로 클라이언트가 넘긴 인벤토리 컴포넌트 포인터가
	// 이 플레이어가 실제로 접근 가능한 대상(본인 인벤토리/창고, 현재 열려있는 상자, 소지 중인 가방의 내부
	// 인벤토리)인지 검증합니다. 이 검사가 없으면 다른 플레이어의 인벤토리 컴포넌트가 리플리케이트된
	// 상황에서 임의의 컴포넌트를 대상으로 아이템을 이동시킬 수 있습니다.
	bool IsInventoryComponentAccessible(UAZInventoryComponent* Comp) const;

	UFUNCTION(Server, Reliable)
	void MoveItem_Server(UAZInventoryComponent* SourceComp, UAZInventoryComponent* DestComp, UAZItemBase* Item, int32 TopLeftIndex, bool bIsRotated);

	//상점 - 서버 권한으로 구매/판매를 처리 (기존에는 UAZStoreSubsystem::BuyItem/SellItem이
	//ServerRPC가 아니라 호출한 클라이언트 로컬에서 그대로 실행되었음)
	UFUNCTION(Server, Reliable)
	void BuyStoreItem_Server(int32 ItemID, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void SellStoreItem_Server(int32 ItemID, int32 Quantity);

	//제작 - 서버 권한으로 시작/수령을 처리 (기존에는 UAZCraftingComponent::TryStartCrafting/
	//CollectCraftedItem이 ServerRPC가 아니라 호출한 클라이언트 로컬에서 그대로 실행되었음)
	UFUNCTION(Server, Reliable)
	void StartCrafting_Server(UAZCraftingComponent* CraftingComponent, int32 JobID);

	UFUNCTION(Server, Reliable)
	void CollectCraftedItem_Server(UAZCraftingComponent* CraftingComponent, int32 JobID);
