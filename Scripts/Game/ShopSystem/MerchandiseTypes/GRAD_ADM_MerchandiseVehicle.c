[BaseContainerProps()]
modded class ADM_MerchandiseVehicle: ADM_MerchandiseType
{	
	override EntitySpawnParams GetVehicleSpawnTransform(ADM_ShopBaseComponent shop)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		if (m_SpawnPosition && !shop.IsInherited(ADM_PhysicalShopComponent)) {
			m_SpawnPosition.GetWorldTransform(params.Transform);
			params.Transform[3] = shop.GetOwner().CoordToParent(params.Transform[3]);
		} else {
			shop.GetOwner().GetTransform(params.Transform);
		}		
		
		bool foundPositionEmpty = false;
        int spawnRadiusMin = 5;
        int spawnRadiusMax = 75;

        while (!foundPositionEmpty) {
            params.Transform[3][0] = params.Transform[3][0] + Math.RandomFloat(-spawnRadiusMin, spawnRadiusMin);
			params.Transform[3][2] = params.Transform[3][2] + Math.RandomFloat(-spawnRadiusMin, spawnRadiusMin);
			params.Transform[3][1] = GetGame().GetWorld().GetSurfaceY(params.Transform[3][0], params.Transform[3][2]);
            foundPositionEmpty = ADM_Utils.IsSpawnPositionClean(m_sPrefab, params, {});
            
            if (spawnRadiusMin < spawnRadiusMax) {
                spawnRadiusMin += 0.25;
				Print(string.Format("OTF - Cant find free position, radius '%1'", spawnRadiusMin), LogLevel.NORMAL);
            }
        }
		
		Print(string.Format("OTF - Found free position at radius  '%1'", spawnRadiusMin), LogLevel.NORMAL);
		
		return params;
	}
}