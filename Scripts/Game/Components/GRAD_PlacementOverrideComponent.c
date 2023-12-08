[ComponentEditorProps(category: "Gruppe Adler/On The Fly", description: "Attach to an entity. Allows to place this entity mit a modified version of the vanilla placement system.")]
class GRAD_PlacementOverrideComponentClass : ScriptComponentClass
{
}

class GRAD_PlacementOverrideComponent : ScriptComponent
{
	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Override Vanilla Placement?", category: "On The Fly")]
	protected bool m_bOverrideVanillaPlacement;
	
	//------------------------------------------------------------------------------------------------
	void SetOverrideVanillaPlacement(bool overrideVanillaPlacement)
	{
		m_bOverrideVanillaPlacement = overrideVanillaPlacement;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetOverrideVanillaPlacement()
	{
		return m_bOverrideVanillaPlacement;
	}
}