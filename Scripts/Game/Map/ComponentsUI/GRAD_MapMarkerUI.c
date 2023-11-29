//------------------------------------------------------------------------------------------------
//! Map line
class MapCircle
{
	float m_fStartPointX, m_fStartPointY;
	float m_fEndPointX, m_fEndPointY;
	Widget m_wRootW;
	Widget m_wCircle;
	ImageWidget m_wCircleImage;
	SCR_MapEntity m_MapEntity;
	GRAD_MapMarkerUI m_OwnerComponent;
	
	//------------------------------------------------------------------------------------------------
	void CreateCircle(notnull Widget rootW)
	{
		m_wRootW = rootW;
		
		Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame)
			return;
		
		m_wCircle = GetGame().GetWorkspace().CreateWidgets("{4B995CEAA55BBECC}UI/Layouts/Map/MapDrawCircle.layout", mapFrame);
		m_wCircleImage = ImageWidget.Cast(m_wCircle.FindAnyWidget("DrawCircleImage"));
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateCircle()
	{	
		if (!m_wCircle)	// can happen due to callater used for update
			return;
				
		int screenX, screenY, endX, endY;

		m_MapEntity.WorldToScreen(m_fStartPointX, m_fStartPointY, screenX, screenY, true);
		m_MapEntity.WorldToScreen(m_fEndPointX, m_fEndPointY, endX, endY, true);
		
		vector circleVector = vector.Zero;
		circleVector[0] = m_fStartPointX - m_fEndPointX;
		circleVector[1] = m_fStartPointY - m_fEndPointY;

		vector angles = circleVector.VectorToAngles();
		if (angles[0] == 90)
			angles[1] =  180 - angles[1]; 	// reverse angles when passing vertical axis
		
		m_wCircleImage.SetRotation(angles[1]);
		
		circleVector = m_MapEntity.GetMapWidget().SizeToPixels(circleVector);
		m_wCircleImage.SetSize(GetGame().GetWorkspace().DPIUnscale(circleVector.Length()), GetGame().GetWorkspace().DPIUnscale(circleVector.Length()));
		
		FrameSlot.SetPos(m_wCircle, GetGame().GetWorkspace().DPIUnscale(screenX), GetGame().GetWorkspace().DPIUnscale(screenY));	// needs unscaled coords
	}
	
	//------------------------------------------------------------------------------------------------
	void MapCircle(SCR_MapEntity mapEnt, GRAD_MapMarkerUI ownerComp)
	{
		m_MapEntity = mapEnt;
		m_OwnerComponent = ownerComp;
	}
};

//------------------------------------------------------------------------------------------------
class GRAD_MapMarkerUI
{
	protected Widget m_wDrawingContainer;
	
	protected ref array<ref MapCircle> m_aCircles = new array <ref MapCircle>();
	
	protected SCR_MapEntity m_MapEntity;
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapPan(float x, float y, bool adjustedPan)
	{
		foreach (MapCircle circle: m_aCircles)
		{
			circle.UpdateCircle();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapPanEnd(float x, float y)
	{
		foreach (MapCircle circle: m_aCircles)
		{
			GetGame().GetCallqueue().CallLater(circle.UpdateCircle, 0, false); // needs to be delayed by a frame as it cant always update the size after zoom correctly within the same frame
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_wDrawingContainer = FrameWidget.Cast(config.RootWidgetRef.FindAnyWidget(SCR_MapConstants.DRAWING_CONTAINER_WIDGET_NAME));
		
		foreach (MapCircle circle: m_aCircles)
		{
			circle.CreateCircle(m_wDrawingContainer);
			GetGame().GetCallqueue().CallLater(circle.UpdateCircle, 0, false);
		}
						
		m_MapEntity.GetOnMapPan().Insert(OnMapPan);		// pan for scaling
		m_MapEntity.GetOnMapPanEnd().Insert(OnMapPanEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		m_MapEntity.GetOnMapPan().Remove(OnMapPan);
		m_MapEntity.GetOnMapPanEnd().Remove(OnMapPanEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	void AddCircle(float startPointX, float startPointY, float endPointX, float endPointY)
	{
		MapCircle circle = new MapCircle(m_MapEntity, this);
		
		circle.m_fStartPointX = startPointX;
		circle.m_fStartPointY = startPointY;
		circle.m_fEndPointX = endPointX;
		circle.m_fEndPointY = endPointY;
		
		m_aCircles.Insert(circle);
	}
	
	//------------------------------------------------------------------------------------------------
	void GRAD_MapMarkerUI()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
};