//------------------------------------------------------------------------------------------------
modded class SCR_ItemPlacementComponent : ScriptComponent
{
	// This modification bypasses the placing animation and adds item rotation in preview
	
	float m_fPlacementRotation = 0;
	GRAD_PlacementOverrideComponent m_PlacementOverrideComponent;
	bool m_bPlacementOverrideEnabled = false;
	IEntity weaponEntity;
	
	//------------------------------------------------------------------------------------------------
	override protected void StartPlaceItem()
	{
		if (!m_bCanPlace)
			return;
		
		IEntity controlledEntity = GetGame().GetPlayerController().GetControlledEntity();
		if (!controlledEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return;
		
		CharacterControllerComponent characterController = character.GetCharacterController();
		if (!characterController)
			return;
		
		characterController.SetDisableWeaponControls(true);
		characterController.SetDisableMovementControls(true);
		
		if (characterController.IsUsingItem())
			return;
		
		vector mat[4];
		Math3D.MatrixCopy(m_vCurrentMat, mat);
		PointInfo ptWS = new PointInfo();
		mat[2] = (mat[3] - character.GetOrigin()).Normalized();
		mat[1] = vector.Up;
		mat[0] = Vector(mat[2][2], mat[2][1], -mat[2][0]);
		ptWS.Set(null, "", mat);
		
		/* ======================================== */
		// modded: disable placing animation
		/*
		if (m_bPlacementOverrideEnabled)
		{
			ItemUseParameters animParams; // added by mod
			OnPlacingEnded(m_EquippedItem, true, animParams); // added by mod
			DisablePreview();
		}
		else
		{
			CharacterAnimationComponent animationComponent = character.GetAnimationComponent();
			int itemActionId = animationComponent.BindCommand("CMD_Item_Action");
			if (characterController.TryUseItemOverrideParams(m_EquippedItem, false, false, itemActionId, 1, 0, 15.0, 1, 0.0, false, ptWS))
			{
				characterController.m_OnItemUseEndedInvoker.Insert(OnPlacingEnded);
				DisablePreview();
			}
		}
		*/
		/* ======================================== */

		characterController.GetAnimationComponent().GetCommandHandler().AlignNewTurns();
		
		OrientToNormal(m_vCurrentMat[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UseXYZPlacement(IEntity owner, float maxPlacementDistance, vector cameraMat[4])
	{
		// Trace against terrain and entities to detect item placement position
		TraceParam param = new TraceParam();
		param.Start = cameraMat[3];
		param.End = param.Start + cameraMat[2] * maxPlacementDistance;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = SCR_PlayerController.GetLocalControlledEntity();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance = world.TraceMove(param, ExcludeWaterCallback);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;
		
		IEntity tracedEntity = param.TraceEnt;
		
		if (traceDistance == 1) // Assume we didn't hit anything and snap item on the ground
		{
			// Trace against terrain and entities to detect new placement position
			TraceParam paramGround = new TraceParam();
			paramGround.Start = param.End + vector.Up;
			paramGround.End = paramGround.Start - vector.Up * 20;
			paramGround.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			paramGround.Exclude = SCR_PlayerController.GetLocalControlledEntity();
			paramGround.LayerMask = EPhysicsLayerPresets.Projectile;
			float traceGroundDistance = world.TraceMove(paramGround, ExcludeWaterCallback);
			m_PreviewEntity.GetTransform(m_vCurrentMat);
			m_vCurrentMat[3] = paramGround.Start + ((paramGround.End - paramGround.Start) * traceGroundDistance) + vector.Up * 0.01; // adding 1 cm to avoid collision with object under
			
			if (traceGroundDistance < 1)
				up = paramGround.TraceNorm;
			
			tracedEntity = paramGround.TraceEnt;
		}
		
		OrientToNormal(up);
		
		m_PreviewEntity.SetTransform(m_vCurrentMat);
		
		/* ======================================== */
		// modded: adding rotation to the preview
		if (m_bPlacementOverrideEnabled)
		{
			vector angles = m_PreviewEntity.GetAngles();
			angles[1] = angles[1] + m_fPlacementRotation;
			m_PreviewEntity.SetAngles(angles);
			m_PreviewEntity.GetTransform(m_vCurrentMat);
		}
		/* ======================================== */
		
		m_PreviewEntity.Update();
		
		IEntity character = SCR_PlayerController.GetLocalControlledEntity();
		vector characterOrigin = character.GetOrigin();
		
		if (Math.AbsFloat(m_vCurrentMat[3][1] - characterOrigin[1] > 0.4)) // Reject based on vertical distance from character
		{
			m_bCanPlace = false;
			return;
		}
		
		if (vector.Distance(m_vCurrentMat[3], characterOrigin) > maxPlacementDistance) // Reject based on distance from character (the maximum should be dictated by item settings)
		{
			m_bCanPlace = false;
			return;
		}
		
		m_bCanPlace = ValidatePlacement(up, tracedEntity, world, SCR_PlayerController.GetLocalControlledEntity());
	}
	
	
	//------------------------------------------------------------------------------------------------
	override protected void DisablePreview()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;

		if (m_bPlacementOverrideEnabled)
		{
			weaponEntity.SetFlags(weaponEntity.GetFlags() | EntityFlags.VISIBLE);
			
			inputManager.ActivateContext("RotatePreviewContext", 0);
			inputManager.RemoveActionListener("RotateLeft", EActionTrigger.DOWN, RotateLeft);
			inputManager.RemoveActionListener("RotateRight", EActionTrigger.DOWN, RotateRight);
		}
		
		super.DisablePreview();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EnablePreview(IEntity weapon)
	{
		m_PlacementOverrideComponent = GRAD_PlacementOverrideComponent.Cast(weapon.FindComponent(GRAD_PlacementOverrideComponent));
		
		weaponEntity = weapon;
		
		if (m_PlacementOverrideComponent)
		{
			m_bPlacementOverrideEnabled = m_PlacementOverrideComponent.GetOverrideVanillaPlacement();
		}
		
		if (m_bPlacementOverrideEnabled)
		{
			weaponEntity.ClearFlags(EntityFlags.VISIBLE);
			
			//weaponEntity.SetScale(0.1); // doesn't work; perhaps because weapon is contineously transformed
			
			InputManager inputManager = GetGame().GetInputManager();
			if (!inputManager)
				return;
	
			inputManager.AddActionListener("RotateLeft", EActionTrigger.DOWN, RotateLeft);
			inputManager.AddActionListener("RotateRight", EActionTrigger.DOWN, RotateRight);
			inputManager.ActivateContext("RotatePreviewContext", 3600000);
		}
		
		super.EnablePreview(weapon);
	}

	//------------------------------------------------------------------------------------------------	
	protected void RotateLeft()
	{
		m_fPlacementRotation += 5;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void RotateRight()
	{
		m_fPlacementRotation -= 5;
	}
};
