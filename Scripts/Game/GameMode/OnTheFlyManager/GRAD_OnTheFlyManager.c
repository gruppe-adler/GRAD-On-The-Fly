enum EOnTheFlyPhase
{
	GAMEMASTER,
	OPFOR,
	BLUFOR,
	GAME,
}

[EntityEditorProps(category: "Gruppe Adler", description: "On The Fly Gamemode Manager")]
class GRAD_OnTheFlyManagerClass : GenericEntityClass
{
}

class GRAD_OnTheFlyManager : GenericEntity
{
	protected static int s_maxCaptureTime = 20;
	
	protected static GRAD_OnTheFlyManager s_Instance;
	
	protected bool m_bOpforSpawnDone;
	protected bool m_bBluforSpawnDone;
	protected bool m_bluforCapturing;
	protected bool m_bluforCaptured;
	protected bool m_winConditionActive;
	protected bool m_debug;
	
	protected int m_bluforCapturingProgress;
	
	protected string m_winnerSide;
	
	protected IEntity m_otfBarrel;
	protected IEntity m_otfBluforSpawnVehicle;
	protected GRAD_BarrelSmokeComponent m_smokeComponent;
	
	protected int m_iOnTheFlyPhase;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// execute only on the server
		if (!Replication.IsServer())
			return;
		
		// check win conditions every second
        GetGame().GetCallqueue().CallLater(CheckWinConditions, 1000, true);
    }
	
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
		if (m_bluforCaptured) {
			Print(string.Format("Skipping CheckBarrelState: Blufor captured!"), LogLevel.NORMAL);
			return;
		}
			
		
		if (!m_otfBarrel) {
			Print(string.Format("Skipping CheckBarrelState: No barrel yet"), LogLevel.NORMAL);
			return;
		}
			
		
		m_smokeComponent = GRAD_BarrelSmokeComponent.Cast(m_otfBarrel.FindComponent(GRAD_BarrelSmokeComponent));
		
		if (!m_smokeComponent) {
			Print(string.Format("Skipping CheckBarrelState: No smoke yet"), LogLevel.NORMAL);
			return;
		}
		
		if (m_smokeComponent.IsSmoking()) {
			m_bluforCapturing = true;
		} else {
			m_bluforCapturing = false;
		}
		
		if (m_bluforCapturing) {
			m_bluforCapturingProgress = m_bluforCapturingProgress + 1;
			Print(string.Format("Capture progress: %1 %", m_bluforCapturingProgress), LogLevel.NORMAL);
		}
		
		if (m_bluforCapturingProgress >= s_maxCaptureTime) {
			m_bluforCaptured = true;
		}
		
		if (!m_bluforCapturing) {
			m_bluforCapturingProgress = 0;
			Print(string.Format("Not capturing: Progress %1 %", m_bluforCapturingProgress), LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckWinConditions()
	{
		if (GetOnTheFlyPhase() != EOnTheFlyPhase.GAME) {
			Print(string.Format("Game not started yet"), LogLevel.NORMAL);
			return;
		}
		
		Print(string.Format("Checking Win Conditions..."), LogLevel.NORMAL);

		// kill loop if win is achieved already
		if (m_winConditionActive) {
			GetGame().GetCallqueue().Remove(CheckWinConditions);
			Print(string.Format("Win Condition Active"), LogLevel.NORMAL);
			return;
		}
		
		// in debug mode we want to test alone without ending the game
		bool bluforEliminated = factionEliminated("US") && !m_debug;
		bool opforEliminated = factionEliminated("USSR") && !m_debug;
		
		checkBarrelState();		
		
		if (bluforEliminated) {
			m_winConditionActive = true;
			m_winnerSide = "opfor";
			Print(string.Format("Blufor eliminated"), LogLevel.NORMAL);
		}
		
		if (opforEliminated || m_bluforCaptured) {
			m_winConditionActive = true;
			m_winnerSide = "blufor";
			Print(string.Format("Opfor eliminated"), LogLevel.NORMAL);
		}
		
		// needs to be on last position as would risk to be overwritten
		if (bluforEliminated && opforEliminated) {
			m_winConditionActive = true;
			m_winnerSide = "draw";
			Print(string.Format("Both sides eliminated"), LogLevel.NORMAL);
		}
		
		if (m_winConditionActive) {
			showGameOver(m_winnerSide);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void showGameOver(string endType) {
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gameMode) {
			Print(string.Format("Gamemode not found in showGameOver"), LogLevel.NORMAL);
			return;
		}
		
		EGameOverTypes gameOverType;
		int winFactionId;
		
		if (endType == "draw")
		{
			gameOverType = EGameOverTypes.FACTION_DRAW;
		}
		
		if (endType == "opfor")
		{
			gameOverType = EGameOverTypes.EDITOR_FACTION_VICTORY;
			winFactionId = 1;
		}
		
		if (endType == "blufor")
		{
			gameOverType = EGameOverTypes.EDITOR_FACTION_VICTORY;
			winFactionId = 0;
		}
		
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(gameOverType, -1, winFactionId);
		gameMode.EndGameMode(endData);
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
			if (!pc)
			{
				// null check bc of null pointer crash
				allPlayers.RemoveItem(playerId);
				continue;
			}
			IEntity controlled = pc.GetControlledEntity();
			if (!controlled)
			{
				// Game Master is also a player but perhaps with no controlled entity
				allPlayers.RemoveItem(playerId);
				continue;
			}
			SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(controlled);
			if (!ch)
			{
				// null check bc of null pointer crash
				allPlayers.RemoveItem(playerId);
				continue;
			}
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
						
			SetOnTheFlyPhase(EOnTheFlyPhase.BLUFOR);
			
			// enter debug mode
			if (isdebug) {
				m_debug = true;
			}			
		} else {
			m_bBluforSpawnDone = true;
			SpawnBluforVehicle(mapPos);
			Print(string.Format("Blufor spawn is done, vehicle created"), LogLevel.NORMAL);
			
			SetOnTheFlyPhase(EOnTheFlyPhase.GAME);
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
		
		m_otfBarrel = barrel;
		
		//Why this line is not printed?
		Print(string.Format("SpawnBarrel executed: %1", m_otfBarrel), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnBluforVehicle(int spawnPosMap[2])
	{
		vector spawnPosition = MapPosToWorldPos(spawnPosMap);
		
		//protected ref RandomGenerator m_pRandomGenerator = new RandomGenerator();
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = spawnPosition;
		
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("bluforSpawnVehicle");
		if (!vehicle)
			return;
		
		m_otfBluforSpawnVehicle = vehicle;
		vehicle.SetOrigin(spawnPosition);		
		
		//Why this line is not printed?
		Print(string.Format("Blufor Spawn Vehicle executed: %1", m_otfBluforSpawnVehicle), LogLevel.NORMAL);
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
	int GetOnTheFlyPhase()
	{
		return m_iOnTheFlyPhase;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnTheFlyPhase(int onTheFlyPhase)
	{
		m_iOnTheFlyPhase = onTheFlyPhase;
		
		switch (m_iOnTheFlyPhase)
		{
			case EOnTheFlyPhase.GAMEMASTER:
				PhaseGameMasterEntered();
				break;
	
			case EOnTheFlyPhase.OPFOR:
				PhaseOpforEntered();
				break;
			
			case EOnTheFlyPhase.BLUFOR:
				PhaseBluforEntered();
				break;
			
			case EOnTheFlyPhase.GAME:
				PhaseGameEntered();
				break;
	
			default:
				Print("OTF - Unknown Phase");
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PhaseGameMasterEntered()
	{
		Print("OTF - Phase 'Game Master' entered", LogLevel.NORMAL)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PhaseOpforEntered()
	{
		Print("OTF - Phase 'Opfor' entered", LogLevel.NORMAL)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PhaseBluforEntered()
	{
		Print("OTF - Phase 'Blufor' entered", LogLevel.NORMAL)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PhaseGameEntered()
	{
		Print("OTF - Phase 'Game' entered", LogLevel.NORMAL)
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
		
		SetEventMask(EntityEvent.INIT);
	}
}