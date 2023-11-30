
![loading](https://github.com/gruppe-adler/GRAD-On-The-Fly/assets/50139270/eedc179b-ee1e-47c6-a5ab-7206e55699d1)

<p align="center">
    <a href="https://github.com/gruppe-adler/GRAD-On-The-Fly/releases/latest">
        <img src="https://img.shields.io/badge/Version-0.0.6-blue.svg?style=flat-square" alt="GRAD On The Fly Version">
    </a>
    <a href="https://www.bistudio.com/community/licenses/arma-public-license-share-alike">
        <img src="https://img.shields.io/badge/License-APL-red.svg?style=flat-square" alt="GRAD On The Fly License">
    </a>
</p>

# On The Fly

## Summary

On The Fly is a simple, dynamic TvT scenario for Arma Reforger, in which OPFOR has to defend a metal barrel while BLUFOR has to capture it. Currently only US vs USSR on Everon are available.

## Dependencies

* [Reforger Lobby](https://reforger.armaplatform.com/workshop/5EAF2B0473DB5A99-ReforgerLobby)
* [Shop System](https://reforger.armaplatform.com/workshop/5D2D1436D1FA5A13-ShopSystem)
* [Placeable Objects](https://reforger.armaplatform.com/workshop/5E654D40F0D628A0-Placeableobjects)

## Recommended Mods

* [Coalition Squad Interface](https://reforger.armaplatform.com/workshop/5B0D1E4380971EBD-CoalitionSquadInterface)

## Game Flow

1. Players are distributed 2:3 between OPFOR and BLUFOR
2. OPFOR commander chooses a starting position on the map
3. OPFOR players are teleported to their chosen location, can move the barrel around and can start setting up their defenses
4. BLUFOR commander chooses a starting position on the map that is outside a specific radius around the OPFOR position (shown on map)
5. BLUFOR players are teleported to their chosen location and can start attacking - the game begins

## Win Conditions

To capture the barrel, a BLUFOR player has to use the barrel action "start smoke". The barrel will start smoking. If an OPFOR player uses the barrel action "stop smoke" then the timer resets.

### OPFOR wins when
* all BLUFOR players are eliminated

### BLUFOR wins when
* all OPFOR players are eliminated OR
* the barrel is captured

## Equipment

Both BLUFOR and OPFOR commander receive money at the start of the game. The buy menu can be opened on the barrel for OPFOR and on the flag for BLUFOR.

Both sides can buy weapons, ammunition, grenades and smoke grenades. OPFOR can buy fortifications and mines. BLUFOR can buy vehicles.

## Links

* [Arma Reforger Workshop](https://reforger.armaplatform.com/workshop/5EE0938694A39CFB-GRADOnTheFly)
* [GitHub](https://github.com/gruppe-adler/GRAD-On-The-Fly)
* [Discord](https://discord.com/invite/ZDqp45q)
* [Website](https://gruppe-adler.de/home)
* [Forum](https://forum.gruppe-adler.de)

## License
GRAD On The Fly is licensed under the ([APL](https://www.bohemia.net/community/licenses/arma-public-license)).
