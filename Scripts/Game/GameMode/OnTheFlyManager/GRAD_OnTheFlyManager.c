[EntityEditorProps(category: "Gruppe Adler", description: "On The Fly Gamemode Manager")]
class GRAD_OnTheFlyManagerClass : GenericEntityClass
{
}

class GRAD_OnTheFlyManager : GenericEntity
{
	protected static GRAD_OnTheFlyManager s_Instance;
	protected bool m_bOpforSpawnDone;
	protected bool m_bBluforSpawnDone;
	protected bool m_bluforCapturing;
	protected int m_bluforCapturingProgress;
	
	protected ParticleEffectEntity m_barrelParticleEffect;
	
	[Attribute("", "Barrel to conquer.", params: "et")]
	protected ResourceName m_sRuinsPrefab;
	
	//------------------------------------------------------------------------------------------------
	bool OpforSpawnDone()
	{
		return m_bOpforSpawnDone;
	}
	
	//------------------------------------------------------------------------------------------------
	bool BluforSpawnDone()
	{
		return m_bBluforSpawnDone;
	}
	
	//------------------------------------------------------------------------------------------------
	bool BluforCapturing()
	{
		return m_bluforCapturing;
	}
	
	//------------------------------------------------------------------------------------------------
	int BluforCapturingProgress()
	{
		return m_bluforCapturingProgress;
	}
	

	
	//------------------------------------------------------------------------------------------------
	void TeleportFactionToMapPos(Faction faction, string factionName, int mapPos[2], bool isdebug)
	{
		if (factionName == "USSR" || isdebug)
		{
			m_bOpforSpawnDone = true;
			SpawnBarrel(mapPos);
			Print(string.Format("Opfor spawn is done, barrel created"), LogLevel.NORMAL);
		}
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return;
			
			if (playerFaction == faction)
			{
				Print(string.Format("Player with ID %1 is Member of Faction %2 and will be teleported to %3", playerId, playerFaction.GetFactionKey(), mapPos), LogLevel.NORMAL);
				playerController.TeleportPlayerToMapPos(playerId, mapPos);
			}
		} 
		

	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyCantTeleportYet(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "Opposing faction did not teleport yet. Wait for the signal.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) == faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyCantTeleportTwice(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can only teleport once. Deal with it.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) == faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyCantTeleportThisFaction(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can only teleport your own faction. Stop trying.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) == faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyOpposingFaction(Faction faction, SCR_MapMarkerBase marker)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "Opposing faction was teleported. Open your map and choose a spawn point.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) != faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
				playerController.InsertLocalMarker(marker);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnBarrel(int spawnPosMap[2])
	{
		vector spawnPosition = MapPosToWorldPos(spawnPosMap);
		
		//protected ref RandomGenerator m_pRandomGenerator = new RandomGenerator();
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = spawnPosition;
		
		Resource resource = Resource.Load("{832EFDAE1C4B9B65}Prefabs/otfBarrel.et");
		IEntity barrel = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		
		//Why this line is not printed?
		Print("SpawnBarrel executed", LogLevel.NORMAL);
		
		// we have only one child so should be fine
		m_barrelParticleEffect = ParticleEffectEntity.Cast(barrel.GetChildren());
		m_barrelParticleEffect.SetDeleteWhenStopped(false);
		m_barrelParticleEffect.Stop;
	}
	
	//------------------------------------------------------------------------------------------------
	vector MapPosToWorldPos(int mapPos[2])
	{
		BaseWorld world = GetGame().GetWorld();
		
		vector worldPos = {mapPos[0], 500, mapPos[1]}; // start tracing in 500m height
				
		vector outDir = {0, -1, 0}; // downward direction
		outDir *= 1000; // trace for 1000 meters
			
		autoptr TraceParam trace = new TraceParam();
		trace.Start = worldPos;
		trace.End = worldPos + outDir;
		trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
		trace.LayerMask = TRACE_LAYER_CAMERA;
		
		float traceDis = world.TraceMove(trace, null);
	        
		vector newWorldPos = worldPos + outDir * traceDis;
		
		return newWorldPos;
	}
	
	//------------------------------------------------------------------------------------------------
	static GRAD_OnTheFlyManager GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	void GRAD_OnTheFlyManager(IEntitySource src, IEntity parent)
	{
		if (s_Instance)
		{
			Print("Only one instance of GRAD_OnTheFlyManager is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}

		s_Instance = this;

		// rest of the init code
	}
}