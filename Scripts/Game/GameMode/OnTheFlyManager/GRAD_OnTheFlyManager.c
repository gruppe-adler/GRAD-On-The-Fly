[EntityEditorProps(category: "Gruppe Adler", description: "On The Fly Gamemode Manager")]
class GRAD_OnTheFlyManagerClass : GenericEntityClass
{
}

class GRAD_OnTheFlyManager : GenericEntity
{
	override void EOnInit(IEntity owner)
    {
		// check win conditions every second
        GetGame().GetCallqueue().CallLater(CheckWinConditions, 1000, true, owner);
    }
	
	protected static int s_maxCaptureTime = 120;
	
	protected static GRAD_OnTheFlyManager s_Instance;
	protected bool m_bOpforSpawnDone;
	protected bool m_bBluforSpawnDone;
	protected bool m_bluforCapturing;
	protected bool m_bluforCaptured;
	protected int m_bluforCapturingProgress;
	protected bool m_winConditionActive;
	protected string m_winnerSide;
	protected IEntity m_otfBarrel;
	protected GRAD_BarrelSmokeComponent m_smokeComponent;
	
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
	bool BluforCaptured()
	{
		return m_bluforCaptured;
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
	
	//---------------------------
	void checkBarrelState()
	{
		if (m_bluforCaptured)
			return;
		
		if (!m_otfBarrel)
			return;
		
		m_smokeComponent = m_otfBarrel.FindComponent(GRAD_BarrelSmokeComponent);
		
		if (!m_smokeComponent) 
			return;
		
		if (m_smokeComponent.IsSmoking()) {
			m_bluforCapturing = true;
		} else {
			m_bluforCapturing = false;
		}
		
		if (m_bluforCapturing) {
			m_bluforCapturingProgress = m_bluforCapturingProgress + 1;
		}
		
		if (m_bluforCapturingProgress >= s_maxCaptureTime) {
			m_bluforCaptured = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckWinConditions()
	{
		// kill loop if win is achieved already
		if (m_winConditionActive) 
			GetGame().GetCallqueue().Remove(CheckWinConditions);
			return;
		
		bool bluforEliminated = factionEliminated("US");
		bool opforEliminated = factionEliminated("USSR");
		
		checkBarrelState();		
		
		if (bluforEliminated && opforEliminated) {
			m_winConditionActive = true;
			m_winnerSide = "draw";
			return;
		}
		
		if (bluforEliminated) {
			m_winConditionActive = true;
			m_winnerSide = "opfor";
			return;
		}
		
		if (opforEliminated || m_bluforCaptured) {
			m_winConditionActive = true;
			m_winnerSide = "blufor";
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void checkCapturing() {
	
	}
		
	//------------------------------------------------------------------------------------------------
	bool factionEliminated(string factionName)
	{
		return (GetAlivePlayersOfSide(factionName) == 0);
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetAlivePlayersOfSide(string factionName)
	{
		int aliveCount = 0;
		
		array<int> allPlayers = {};
		GetGame().GetPlayerManager().GetPlayers(allPlayers);
		foreach(int playerId : allPlayers)
		{
		  PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		  IEntity controlled = pc.GetControlledEntity();
		  SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(controlled);
		  CharacterControllerComponent ccc = ch.GetCharacterController();
		  if (factionName != ch.GetFactionKey() || ccc.IsDead()) allPlayers.RemoveItem(playerId);
		}
		aliveCount = allPlayers.Count();
		
		return aliveCount;
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