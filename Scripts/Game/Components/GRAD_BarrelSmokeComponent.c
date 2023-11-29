[ComponentEditorProps(category: "Gruppe Adler/On The Fly", description: "Attach to the barrel to enable/disable smoke.")]
class GRAD_BarrelSmokeComponentClass : ScriptComponentClass
{
}

class GRAD_BarrelSmokeComponent : ScriptComponent
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourceNamePicker, desc: "Smoke effect", params: "ptc", precision: 3, category: "On The Fly")]
	protected ResourceName m_sSmokeParticle;
	
	private ParticleEffectEntity m_pSmokeParticle;
	
	private bool m_bIsSmoking;
	
	//------------------------------------------------------------------------------------------------
	void EnableSmoke()
	{
		if (!m_pSmokeParticle) 
		{
			// Create particle emitter
			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			spawnParams.TargetWorld = GetOwner().GetWorld();
			spawnParams.Parent = GetOwner();
			Math3D.MatrixIdentity4(spawnParams.Transform);
			spawnParams.Transform[3] = "0.0 0.7 0.0";
			m_pSmokeParticle = ParticleEffectEntity.SpawnParticleEffect(m_sSmokeParticle, spawnParams);
		}
		else
		{
			m_pSmokeParticle.Play();
		}
		
		m_bIsSmoking = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void DisableSmoke()
	{
		if (m_pSmokeParticle)
		{
			m_pSmokeParticle.Stop();
		}
		
		m_bIsSmoking = false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsSmoking()
	{
		return m_bIsSmoking;
	}
}