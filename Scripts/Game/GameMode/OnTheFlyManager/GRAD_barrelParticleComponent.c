class GRAD_BarrelParticleComponentClass : ScriptComponentClass
{
};

class GRAD_BarrelParticleComponent  : ScriptComponent
{
	
	private ParticleEffectEntity m_pParticle;
	
    override void EOnInit(IEntity owner)
    {
		super.EOnInit(owner);
        Print("EOnInit BarrelParticleComponent");
		
		[Attribute("{5EE0938694A39CFB}Particles/barrelSmoke.ptc", UIWidgets.ResourcePickerThumbnail, "Prefab of particle.", category: "Zone", params: "ptc")]
    	protected ResourceName m_sParticle;    
	
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.TargetWorld = GetOwner().GetWorld();
		spawnParams.Parent = owner;
		Math3D.MatrixIdentity4(spawnParams.Transform);
    
		m_pParticle = ParticleEffectEntity.SpawnParticleEffect(m_sParticle, spawnParams);
   
    }
	
};