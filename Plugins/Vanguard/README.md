# VANGUARD - Rivalry & Dynamic Encounter System

A next-generation UE5.6-ready plugin for creating personalized adversaries and dynamic encounters in story-driven RPGs, open-world action-adventure games, and shooters.

## Overview

VANGUARD provides a comprehensive system for creating memorable rivals that evolve based on player actions, dynamic encounters that feel authored but emerge naturally, and faction relationships that shape the game world.

### Key Features

- **Personalized Adversaries**: Rivals remember the player, evolve over time, and develop unique traits based on encounters
- **Dynamic Encounters**: Systemic encounters including patrols, ambushes, convoys, rescues, and sieges
- **Faction System**: Complex relationships between factions with hostility matrices and territory control
- **Cross-Genre Support**: Works for melee/RPG and shooter gameplay loops
- **Narrative Integration**: Plugs into quest systems without railroading emergent gameplay
- **Scalable Architecture**: From named rivals to hundreds of threats using efficient systems
- **Deterministic Persistence**: Everything saves/loads safely across sessions

## Architecture

### Core Systems

#### Data Assets (Primary Data Assets)
- **URivalArchetype**: Defines rival character types with identity, AI behavior, and growth rules
- **UEncounterArchetype**: Templates for different encounter types (ambush, convoy, siege, etc.)
- **UFactionArchetype**: Faction properties, hostility relationships, and territorial behavior
- **UTraitArchetype**: Traits that can be assigned to rivals (Pyromaniac, Cowardly Sniper, etc.)

#### Components
- **URivalProfileComponent**: Embeds rival identity, history, injuries, and rivalry state
- **UFactionAffiliationComponent**: Provides team affiliation for AI perception and hostility logic
- **UEncounterHandleComponent**: Tracks actor's relationship to current encounter

#### Subsystems
- **URivalSystem** (GameInstance): Global registry of rivals, factions, and relationships
- **UEncounterDirectorSubsystem** (World): Per-world encounter orchestration and budget management
- **UEncounterDebugSubsystem** (Engine): Debug visualization and performance monitoring

#### AI Integration
- **AVanguardAIController**: Enhanced AI controller with rivalry awareness and tactical adaptation
- Integration with UE5's AI Perception, EQS, and Smart Objects systems
- Support for both Behavior Trees and State Trees

#### Save/Load System
- **URivalSaveGame**: Comprehensive save system for rival rosters, faction relationships, and world state
- Schema versioning and migration support
- Atomic save operations with integrity validation

## Getting Started

### Installation

1. Copy the `Vanguard` plugin folder to your project's `Plugins` directory
2. Add `"Vanguard"` to your project's `.uproject` file dependencies
3. Regenerate project files and compile

### Basic Setup

1. **Enable Required Plugins**: Ensure GameplayAbilities, AIModule, NavigationSystem, and GameplayTags are enabled
2. **Create Archetypes**: Create your first rival and encounter archetypes as Primary Data Assets
3. **Setup Gameplay Tags**: Create gameplay tag hierarchies for traits, factions, and encounter types
4. **Configure AI**: Use VanguardAIController for enhanced AI behavior

### Example Usage

```cpp
// Register a rival
if (URivalSystem* RivalSystem = GetGameInstance()->GetSubsystem<URivalSystem>())
{
    RivalSystem->RegisterRival(MyRivalActor);
}

// Schedule an encounter
if (UEncounterDirectorSubsystem* EncounterDirector = GetWorld()->GetSubsystem<UEncounterDirectorSubsystem>())
{
    FGuid EncounterId;
    EncounterDirector->TryScheduleEncounter(
        FGameplayTag::RequestGameplayTag(TEXT("Encounter.Type.Ambush")),
        PlayerLocation,
        EncounterId
    );
}

// React to player damage
if (AVanguardAIController* VanguardAI = Cast<AVanguardAIController>(GetController()))
{
    VanguardAI->ReactToPlayerDamage(DamageAmount, DamageType);
}
```

## Gameplay Tags Hierarchy

### Recommended Tag Structure

```
Vanguard
├── Faction
│   ├── Crimson
│   ├── Azure
│   └── Neutral
├── Trait
│   ├── Combat
│   │   ├── Aggressive
│   │   ├── Defensive
│   │   └── Berserker
│   ├── Stealth
│   │   ├── Camouflage
│   │   ├── Silent
│   │   └── Tracker
│   └── Social
│       ├── Leader
│       ├── Cowardly
│       └── Loyal
├── Encounter
│   ├── Type
│   │   ├── Patrol
│   │   ├── Ambush
│   │   ├── Convoy
│   │   ├── Siege
│   │   └── Rescue
│   └── State
│       ├── Active
│       ├── Retreating
│       └── Completed
└── AI
    ├── Tactic
    │   ├── Patrol
    │   ├── Assault
    │   ├── Flank
    │   └── Retreat
    └── State
        ├── Idle
        ├── Alert
        └── Combat
```

## Debug Console Commands

- `vanguard.debug [feature]` - Toggle debug visualization
- `vanguard.stats` - Show performance statistics  
- `vanguard.clear` - Clear debug data
- `vanguard.dump` - Dump debug events to log

### Debug Features
- `RivalProfiles` - Show rival information
- `EncounterAreas` - Show encounter zones
- `FactionRelationships` - Show faction connections
- `Performance` - Show performance metrics
- `Cooldowns` - Show cooldown areas

## Performance Considerations

### Budget System
- Encounters consume budget based on complexity
- Automatic budget management prevents performance spikes
- Time-sliced operations for smooth frame rates

### Memory Management
- Weak references prevent memory leaks
- Automatic cleanup of invalid actors
- Configurable event history limits

### Scalability
- Support for Mass/ECS integration for large populations
- World Partition awareness for streaming worlds
- Asset Manager integration for efficient loading

## Extending the System

### Custom Rival Archetypes

```cpp
UCLASS()
class YOURGAME_API UMyRivalArchetype : public URivalArchetype
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float CustomProperty = 1.0f;
};
```

### Custom Encounter Types

```cpp
UCLASS()
class YOURGAME_API UMyEncounterArchetype : public UEncounterArchetype
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FVector> SpawnPoints;
};
```

### Custom AI Behavior

```cpp
UCLASS()
class YOURGAME_API AMyVanguardAI : public AVanguardAIController
{
    GENERATED_BODY()
    
protected:
    virtual void UpdateTacticalBehavior(float DeltaTime) override;
};
```

## Integration with Other Systems

### Gameplay Abilities System (GAS)
- Traits can grant gameplay effects and abilities
- Rival progression through ability acquisition
- Player rewards via ability unlocks

### Narrative Systems
- Story gates can influence encounter spawning
- Quest integration without breaking emergence
- Rival cameos during story beats

### Multiplayer Support
- Server-authoritative rival state
- Shared notoriety vs per-player vendettas
- Join-in-progress support

## Best Practices

### Design Guidelines
1. **Start Small**: Begin with a few rival archetypes and expand
2. **Balance Emergence**: Don't over-constrain the system
3. **Test Persistence**: Regularly test save/load functionality
4. **Monitor Performance**: Use debug tools to track resource usage
5. **Iterate on Feedback**: Adjust based on player behavior patterns

### Common Pitfalls
- Over-aggressive encounter spawning
- Insufficient cooldown periods
- Memory leaks from strong references
- Unbalanced faction relationships

## Troubleshooting

### Common Issues

**Rivals not saving**: Check that URivalProfileComponent has valid GUID
**Encounters not spawning**: Verify budget availability and location cooldowns
**AI not reacting**: Ensure components are properly configured
**Performance issues**: Monitor budget usage and event history size

### Debug Tools
Use the built-in debug subsystem to visualize system state and identify issues.

## Roadmap

### Near Term
- Enhanced EQS integration
- More encounter types
- Territory control visualization
- Blueprint function library

### Future
- Mass Entity integration
- Procedural rival generation
- Advanced faction diplomacy
- Editor tools and wizards

## License

This plugin is provided as-is for educational and development purposes. Please ensure compatibility with your project's licensing requirements.

## Support

For issues, feature requests, or questions about VANGUARD, please refer to the documentation or contact the development team.

---

*VANGUARD - Creating memorable adversaries, one encounter at a time.*