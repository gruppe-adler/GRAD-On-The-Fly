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

// This class is server-only code

class GRAD_OnTheFlyManager : GenericEntity
{
	[Attribute(defvalue: "1000", uiwidget: UIWidgets.Slider, enums: NULL, desc: "How long in milliseconds is the win conditions check interval.", category: "On The Fly - Parameters", params: "1000 60000 1000")]
	protected int m_iCheckInterval;
	
	[Attribute(defvalue: "20", uiwidget: UIWidgets.Slider, enums: NULL, desc: "How long in check cycles the barrel needs to be captured by BLUFOR to win.", category: "On The Fly - Parameters", params: "1 100 1")]
	protected int m_iMaxCaptureTime;
	
	[Attribute(defvalue: "1000", uiwidget: UIWidgets.Slider, enums: NULL, desc: "How far away BLUFOR needs to spawn from OPFOR barrel.", category: "On The Fly - Parameters", params: "1 5000 1")]
	protected int m_iBluforSpawnDistance;
	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, enums: NULL, desc: "How long in seconds the notifications should be displayed", category: "On The Fly - Parameters", params: "1 30 1")]
	protected int m_iNotificationDuration;
	
	[Attribute(defvalue: "1000", uiwidget: UIWidgets.Slider, enums: NULL, desc: "How long should the tracing distance be to convert map in world coordinates", category: "On The Fly - Parameters", params: "1.0 5000.0 1.0")]
	protected float m_iTracingDistance;
	
	protected vector m_OpforSpawnPos;
	protected vector m_BluforSpawnPos;
	
	protected bool m_bluforCapturing;
	protected bool m_bluforCaptured;
	protected bool m_winConditionActive;
	protected bool m_debug;
	
	protected int m_bluforCapturingProgress;
	
	protected string m_winnerSide;
	
	protected IEntity m_otfBarrel;
	protected IEntity m_otfBluforSpawnVehicle;
	protected GRAD_BarrelSmokeComponent m_smokeComponent;
	
	protected const int MARKER_RADIUS = 1000;
	
	[RplProp()]
	protected int m_iOnTheFlyPhase;
	
	protected static GRAD_OnTheFlyManager s_Instance;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// execute only on the server
		if (!Replication.IsServer())
			return;
		
		// check win conditions every second
        GetGame().GetCallqueue().CallLater(CheckWinConditions, m_iCheckInterval, true);
    }
	
	//------------------------------------------------------------------------------------------------
	void OnSynchedMarkerAdded(SCR_MapMarkerBase marker)
	{
		if (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM)
			return; // only custom markers will have marker text that we need
		
		string markerText = marker.GetCustomText();
		Print(string.Format("OTF - Custom Marker '%1' placed.", markerText), LogLevel.NORMAL);
		markerText.ToLower();
		if (!(markerText == "debug" || markerText == "opfor" || markerText == "blufor"))
			return; // we are only interested in these special markers
		
		int markerOwnerId = marker.GetMarkerOwnerID();
		
		// only characters with the role 'OTF Commander' either BLUFOR or OPFOR are allowed to create teleport markers
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(markerOwnerId));
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		GRAD_CharacterRoleComponent characterRoleComponent = GRAD_CharacterRoleComponent.Cast(ch.FindComponent(GRAD_CharacterRoleComponent));
		string characterRole = characterRoleComponent.GetCharacterRole();
		if (characterRole != "OTF Commander")
		{
			Print(string.Format("OTF - Wrong role for marker. Current Role '%1'", characterRole), LogLevel.NORMAL);
			NotifyPlayerWrongRole(markerOwnerId, "OTF Commander");
			return;
		}
		
		int markerPos[2];
		marker.GetWorldPos(markerPos);
		
		Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(markerOwnerId);
		
		switch (markerText)
		{
			case "debug":
				DebugMarkerCreated(marker, markerPos, markerOwnerFaction);
				break;
	
			case "opfor":
				OpforMarkerCreated(marker, markerPos, markerOwnerFaction);
				break;
			
			case "blufor":
				BluforMarkerCreated(marker, markerPos, markerOwnerFaction);
				break;
		}
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
	
	//------------------------------------------------------------------------------------------------
	void checkBarrelState()
	{
		if (m_bluforCaptured) {
			Print(string.Format("OTF - Skipping CheckBarrelState: Blufor captured!"), LogLevel.NORMAL);
			return;
		}
			
		
		if (!m_otfBarrel) {
			Print(string.Format("OTF - Skipping CheckBarrelState: No barrel yet"), LogLevel.NORMAL);
			return;
		}
			
		
		m_smokeComponent = GRAD_BarrelSmokeComponent.Cast(m_otfBarrel.FindComponent(GRAD_BarrelSmokeComponent));
		
		if (!m_smokeComponent) {
			Print(string.Format("OTF - Skipping CheckBarrelState: smoke component missing"), LogLevel.NORMAL);
			return;
		}
		
		if (m_smokeComponent.IsSmoking()) {
			m_bluforCapturing = true;
		} else {
			m_bluforCapturing = false;
		}
		
		if (m_bluforCapturing) {
			m_bluforCapturingProgress = m_bluforCapturingProgress + 1;
			Print(string.Format("OTF - Capture progress: %1 %", m_bluforCapturingProgress), LogLevel.NORMAL);
		}
		
		if (m_bluforCapturingProgress >= m_iMaxCaptureTime) {
			m_bluforCaptured = true;
		}
		
		if (!m_bluforCapturing) {
			m_bluforCapturingProgress = 0;
			Print(string.Format("OTF - Not capturing: Progress %1 %", m_bluforCapturingProgress), LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckWinConditions()
	{
		if (GetOnTheFlyPhase() != EOnTheFlyPhase.GAME) {
			Print(string.Format("OTF - Game not started yet"), LogLevel.NORMAL);
			return;
		}
		
		Print(string.Format("OTF - Checking Win Conditions..."), LogLevel.NORMAL);

		// kill loop if win is achieved already
		if (m_winConditionActive) {
			GetGame().GetCallqueue().Remove(CheckWinConditions);
			Print(string.Format("OTF - Win Condition Active"), LogLevel.NORMAL);
			return;
		}
		
		// in debug mode we want to test alone without ending the game
		bool bluforEliminated = factionEliminated("US") && !m_debug;
		bool opforEliminated = factionEliminated("USSR") && !m_debug;
		
		checkBarrelState();		
		
		if (bluforEliminated) {
			m_winConditionActive = true;
			m_winnerSide = "opfor";
			Print(string.Format("OTF - Blufor eliminated"), LogLevel.NORMAL);
		}
		
		if (opforEliminated || m_bluforCaptured) {
			m_winConditionActive = true;
			m_winnerSide = "blufor";
			Print(string.Format("OTF - Opfor eliminated"), LogLevel.NORMAL);
		}
		
		// needs to be on last position as would risk to be overwritten
		if (bluforEliminated && opforEliminated) {
			m_winConditionActive = true;
			m_winnerSide = "draw";
			Print(string.Format("OTF - Both sides eliminated"), LogLevel.NORMAL);
		}
		
		if (m_winConditionActive) {
			showGameOver(m_winnerSide);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void showGameOver(string endType) {
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!gameMode) {
			Print(string.Format("OTF - Gamemode not found in showGameOver"), LogLevel.NORMAL);
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
			winFactionId = GetFactionIdByFactionKey("USSR");
		}
		
		if (endType == "blufor")
		{
			gameOverType = EGameOverTypes.EDITOR_FACTION_VICTORY;
			winFactionId = GetFactionIdByFactionKey("US");
		}
		
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(gameOverType, -1, winFactionId);
		gameMode.EndGameMode(endData);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionIdByFactionKey(string factionKey)
	{
		int factionId = -1;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		SCR_SortedArray<SCR_Faction> outFactions = new SCR_SortedArray<SCR_Faction>();
		factionManager.GetSortedFactionsList(outFactions);
		
		Print(outFactions.Count());
		
		for (int i = 0; i < outFactions.Count(); i++)
		{
			if (outFactions[i].GetFactionKey() == factionKey)
				factionId = factionManager.GetFactionIndex(outFactions[i]);
		}
		
		PrintFormat("OTF - factionKey: %1 factionId: %2", factionKey, factionId);
		
		return factionId;
	}
		
	//------------------------------------------------------------------------------------------------
	bool factionEliminated(string factionName)
	{
		return (GetAlivePlayersOfSide(factionName) == 0);
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetAlivePlayersOfSide(string factionName)
	{
		array<int> alivePlayersOfSide = {};
		
		array<int> allPlayers = {};
		GetGame().GetPlayerManager().GetPlayers(allPlayers);
		foreach(int playerId : allPlayers)
		{
			// null check bc of null pointer crash
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (!pc) continue;
			
			// Game Master is also a player but perhaps with no controlled entity
			IEntity controlled = pc.GetControlledEntity();
			if (!controlled) continue;
			
			// null check bc of null pointer crash
			SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(controlled);
			if (!ch) continue;
			
			CharacterControllerComponent ccc = ch.GetCharacterController();
			if (factionName != ch.GetFactionKey() || ccc.IsDead()) continue;
			
			alivePlayersOfSide.Insert(playerId);
		}
		
		return alivePlayersOfSide.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	void TeleportFactionToMapPos(Faction faction, string factionName, int mapPos[2], bool isdebug)
	{
		if (factionName == "USSR" || isdebug)
		{
			vector worldPos = {mapPos[0], 0, mapPos[1]}; 		
			m_OpforSpawnPos = worldPos;
			SpawnBarrel(mapPos);
			Print(string.Format("OTF - Opfor spawn is done, barrel created"), LogLevel.NORMAL);
						
			// enter debug mode
			if (isdebug) {
				m_debug = true;
			}			
		} else {
			SpawnBluforVehicle(mapPos);
			Print(string.Format("OTF - Blufor spawn is done, spawn vehicle moved"), LogLevel.NORMAL);
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
				Print(string.Format("OTF - Player with ID %1 is Member of Faction %2 and will be teleported to %3", playerId, playerFaction.GetFactionKey(), mapPos), LogLevel.NORMAL);
				playerController.TeleportPlayerToMapPos(playerId, mapPos);
			}
		} 
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsBluforSpawnDistanceToShort(int mapPos[2])
	{
		vector worldPos = {mapPos[0], 0, mapPos[1]};
		if (worldPos.Distance(worldPos,m_OpforSpawnPos) < m_iBluforSpawnDistance)
			return true;
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyFactionCantTeleportYet(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "Opposing faction did not teleport yet. Wait for the signal.";
		int duration = m_iNotificationDuration;
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
	void NotifyFactionCantTeleportTwice(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can only teleport once. Deal with it.";
		int duration = m_iNotificationDuration;
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
	void NotifyFactionSpawnTooClose(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You need to be at least 1000m away from enemy to spawn.";
		int duration = m_iNotificationDuration;
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
	void NotifyFactionCantTeleport(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can only teleport your own faction. Stop trying.";
		int duration = m_iNotificationDuration;
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
	void NotifyFactionWrongPhaseForMarker(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can't create the marker in this phase.";
		int duration = m_iNotificationDuration;
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
	void NotifyOpposingFactionAfterOpforPhase(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "OPFOR was teleported. Open your map and choose a spawn point.";
		int duration = m_iNotificationDuration;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) != faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyOpposingFactionAfterBluforPhase(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "BLUFOR was teleported. The Battle begins.";
		int duration = m_iNotificationDuration;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) != faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyAllOnPhaseChange(int phase)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = string.Format("New phase '%1' entered.", SCR_Enum.GetEnumName(EOnTheFlyPhase, phase));
		int duration = m_iNotificationDuration;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return;
		
			playerController.ShowHint(message, title, duration, isSilent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyPlayerWrongRole(int playerId, string neededRole)
	{

		string title = "On The Fly";
		string message = string.Format("You have the wrong role to create a teleport marker. You need to have the '%1' role.", neededRole);
		int duration = m_iNotificationDuration;
		bool isSilent = false;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		
		if (!playerController)
			return;
	
		playerController.ShowHint(message, title, duration, isSilent);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddMarkerToOpposingFaction(Faction faction, SCR_MapMarkerBase marker)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) != faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.InsertLocalMarker(marker);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddBarrelMarkerToAllPlayers()
	{
		vector barrelWorldPos = m_otfBarrel.GetOrigin();
		
		SCR_MapMarkerBase barrelMarker = new SCR_MapMarkerBase();
		barrelMarker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
		barrelMarker.SetWorldPos(barrelWorldPos[0], barrelWorldPos[2]);
		barrelMarker.SetCustomText("barrel");
		barrelMarker.SetColorEntry(4); // should be red
		barrelMarker.SetIconEntry(11); // should be flag
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{

			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return;
		
			playerController.InsertLocalMarker(barrelMarker);
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddBarrelSpawnRadiusMarkerToAllPlayers()
	{
		vector barrelWorldPos = m_otfBarrel.GetOrigin();
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{

			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return;
		
			playerController.AddCircleMarker(barrelWorldPos[0] - MARKER_RADIUS, barrelWorldPos[2] + MARKER_RADIUS, barrelWorldPos[0] + MARKER_RADIUS, barrelWorldPos[2] + MARKER_RADIUS);
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
		Print(string.Format("OTF - SpawnBarrel executed: %1", m_otfBarrel), LogLevel.NORMAL);
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
		vehicle.SetOrigin(spawnPosition); // just move for now, later might be created dynamically
		// TODO: Changing the position of the vehicle on the server is not synced to the clients
		// If GM moves the vehicle by hand the position is synced
		
		//Why this line is not printed?
		Print(string.Format("OTF - Blufor Spawn Vehicle executed: %1", m_otfBluforSpawnVehicle), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	vector MapPosToWorldPos(int mapPos[2])
	{
		BaseWorld world = GetGame().GetWorld();
		
		vector worldPos = {mapPos[0], m_iTracingDistance / 2, mapPos[1]}; // start tracing in half tracing height height
				
		vector outDir = {0, -1, 0}; // downward direction
		outDir *= m_iTracingDistance; // trace for x meters
			
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
	void SetOnTheFlyPhase(int phase)
	{
		Rpc(RpcAsk_Authority_SetOnTheFlyPhase, phase);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void 	RpcAsk_Authority_SetOnTheFlyPhase(int phase)
	{
		m_iOnTheFlyPhase = phase;
		
		Replication.BumpMe();
		
		NotifyAllOnPhaseChange(phase);
		
		Print(string.Format("OTF - Phase '%1' entered (%2)", SCR_Enum.GetEnumName(EOnTheFlyPhase, phase), phase), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	void DebugMarkerCreated(SCR_MapMarkerBase marker, int markerPos[2], Faction markerOwnerFaction)
	{
		// allow debug spawn without peer tool
		TeleportFactionToMapPos(markerOwnerFaction, markerOwnerFaction.GetFactionKey(), markerPos, true);
		AddMarkerToOpposingFaction(markerOwnerFaction, marker);
		SetOnTheFlyPhase(EOnTheFlyPhase.GAME);
	}
	
	//------------------------------------------------------------------------------------------------
	void OpforMarkerCreated(SCR_MapMarkerBase marker, int markerPos[2], Faction markerOwnerFaction)
	{
		// manage opfor marker placement
		if (m_iOnTheFlyPhase != EOnTheFlyPhase.OPFOR) {
			NotifyFactionWrongPhaseForMarker(markerOwnerFaction);
			return;
		}
			
		if (markerOwnerFaction.GetFactionKey() == "USSR") {	
			TeleportFactionToMapPos(markerOwnerFaction, markerOwnerFaction.GetFactionKey(), markerPos, false);
			// This will now be done via barrel action
			// Also the barrel action will add the barrel marker to all players
			//SetOnTheFlyPhase(EOnTheFlyPhase.BLUFOR);
			
			//AddMarkerToOpposingFaction(markerOwnerFaction, marker);
			//GetGame().GetCallqueue().CallLater(NotifyOpposingFactionAfterOpforPhase, m_iNotificationDuration, false, markerOwnerFaction);
		} else {
			NotifyFactionCantTeleport(markerOwnerFaction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void BluforMarkerCreated(SCR_MapMarkerBase marker, int markerPos[2], Faction markerOwnerFaction)
	{
		// manage blufor marker placement
		if (m_iOnTheFlyPhase != EOnTheFlyPhase.BLUFOR) {
			NotifyFactionWrongPhaseForMarker(markerOwnerFaction);
			return;
		}
		
		if (IsBluforSpawnDistanceToShort(markerPos)) {
			NotifyFactionSpawnTooClose(markerOwnerFaction);
			return;
		}
			
		if (markerOwnerFaction.GetFactionKey() == "US") {
			TeleportFactionToMapPos(markerOwnerFaction, markerOwnerFaction.GetFactionKey(), markerPos, false);
			SetOnTheFlyPhase(EOnTheFlyPhase.GAME);
			
        	GetGame().GetCallqueue().CallLater(NotifyOpposingFactionAfterBluforPhase, m_iNotificationDuration, false, markerOwnerFaction);
		} else {
			NotifyFactionCantTeleport(markerOwnerFaction);
		}
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
			Print("OTF - Only one instance of GRAD_OnTheFlyManager is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}

		s_Instance = this;

		// rest of the init code
		
		SetEventMask(EntityEvent.INIT);
	}
}