#ifndef GUARD_CONSTANTS_WEATHER_H
#define GUARD_CONSTANTS_WEATHER_H

#define WEATHER_NONE                    0
#define WEATHER_SUNNY_CLOUDS            1
#define WEATHER_SUNNY                   2
#define WEATHER_RAIN                    3
#define WEATHER_SNOW                    4   // Unused
#define WEATHER_RAIN_THUNDERSTORM       5
#define WEATHER_FOG_HORIZONTAL          6
#define WEATHER_VOLCANIC_ASH            7
#define WEATHER_SANDSTORM               8
#define WEATHER_FOG_DIAGONAL            9   // Unused
#define WEATHER_UNDERWATER              10  // Unused
#define WEATHER_SHADE                   11  // Original name was closer to WEATHER_CLOUDY/OVERCAST
#define WEATHER_DROUGHT                 12
#define WEATHER_DOWNPOUR                13
#define WEATHER_UNDERWATER_BUBBLES      14
#define WEATHER_ABNORMAL                15  // The alternating weather during Groudon/Kyogre conflict
#define WEATHER_ROUTE119_CYCLE          20
#define WEATHER_ROUTE123_CYCLE          21

// Dynamic weather (src/dynamic_weather.c): a map in a zone gets the weather the zone has
// today in the current day/night phase. A zone is a group of neighbouring maps that always
// share the weather, so it doesn't change at every map border.
#define DYN_ZONE_VIRIDIAN               0   // Pallet Town, Route 1, Viridian City, Route 2, Route 22, the Test Area
#define DYN_ZONE_PEWTER                 1   // Pewter City, Route 3
#define DYN_ZONE_CERULEAN               2   // Route 4, Cerulean City, Routes 24, 25, 5, 9
#define DYN_ZONE_VERMILION              3   // Vermilion City, Routes 6, 11
#define DYN_ZONE_LAVENDER               4   // Lavender Town, Routes 8, 10, 12
#define DYN_ZONE_CELADON                5   // Celadon City, Saffron City, Routes 7, 16, 17, 18
#define DYN_ZONE_FUCHSIA                6   // Fuchsia City, Routes 13, 14, 15, the Safari Zone
#define DYN_ZONE_SEA                    7   // Routes 19, 20, 21, Cinnabar Island
#define DYN_ZONE_INDIGO                 8   // Route 23, Indigo Plateau
#define DYN_ZONE_SEVII_NORTH            9   // One, Two and Three Island and their routes
#define DYN_ZONE_FOUR_ISLAND            10  // Four Island (no map connections, so its snow has no border)
#define DYN_ZONE_SEVII_FOUR_FIVE        11  // Five Island and its routes
#define DYN_ZONE_SEVII_SOUTH            12  // Six and Seven Island and their routes
#define DYN_ZONE_MEGA_ISLAND            13  // Mega Island, its port and the castle grounds
#define DYN_ZONE_COUNT                  14

#define WEATHER_DYNAMIC_START           32
#define WEATHER_DYNAMIC_VIRIDIAN        (WEATHER_DYNAMIC_START + DYN_ZONE_VIRIDIAN)
#define WEATHER_DYNAMIC_PEWTER          (WEATHER_DYNAMIC_START + DYN_ZONE_PEWTER)
#define WEATHER_DYNAMIC_CERULEAN        (WEATHER_DYNAMIC_START + DYN_ZONE_CERULEAN)
#define WEATHER_DYNAMIC_VERMILION       (WEATHER_DYNAMIC_START + DYN_ZONE_VERMILION)
#define WEATHER_DYNAMIC_LAVENDER        (WEATHER_DYNAMIC_START + DYN_ZONE_LAVENDER)
#define WEATHER_DYNAMIC_CELADON         (WEATHER_DYNAMIC_START + DYN_ZONE_CELADON)
#define WEATHER_DYNAMIC_FUCHSIA         (WEATHER_DYNAMIC_START + DYN_ZONE_FUCHSIA)
#define WEATHER_DYNAMIC_SEA             (WEATHER_DYNAMIC_START + DYN_ZONE_SEA)
#define WEATHER_DYNAMIC_INDIGO          (WEATHER_DYNAMIC_START + DYN_ZONE_INDIGO)
#define WEATHER_DYNAMIC_SEVII_NORTH     (WEATHER_DYNAMIC_START + DYN_ZONE_SEVII_NORTH)
#define WEATHER_DYNAMIC_FOUR_ISLAND     (WEATHER_DYNAMIC_START + DYN_ZONE_FOUR_ISLAND)
#define WEATHER_DYNAMIC_SEVII_FOUR_FIVE (WEATHER_DYNAMIC_START + DYN_ZONE_SEVII_FOUR_FIVE)
#define WEATHER_DYNAMIC_SEVII_SOUTH     (WEATHER_DYNAMIC_START + DYN_ZONE_SEVII_SOUTH)
#define WEATHER_DYNAMIC_MEGA_ISLAND     (WEATHER_DYNAMIC_START + DYN_ZONE_MEGA_ISLAND)
#define WEATHER_DYNAMIC_END             (WEATHER_DYNAMIC_START + DYN_ZONE_COUNT)

// The weather forecasters (data/scripts/weather_forecaster.inc), by the zones they cover
#define FORECAST_VIRIDIAN               0   // Viridian, Pewter, Indigo
#define FORECAST_CERULEAN               1   // Cerulean, Vermilion, Lavender, Mega Island
#define FORECAST_CELADON                2   // Celadon, Fuchsia, the sea
#define FORECAST_ONE_ISLAND             3   // The Sevii Islands
#define FORECAST_COUNT                  4

// These are used in maps' coord_weather_event entries.
// They are not a one-to-one mapping with the engine's
// internal weather constants above.
#define COORD_EVENT_WEATHER_SUNNY_CLOUDS        1
#define COORD_EVENT_WEATHER_SUNNY               2
#define COORD_EVENT_WEATHER_RAIN                3
#define COORD_EVENT_WEATHER_SNOW                4
#define COORD_EVENT_WEATHER_RAIN_THUNDERSTORM   5
#define COORD_EVENT_WEATHER_FOG_HORIZONTAL      6
#define COORD_EVENT_WEATHER_FOG_DIAGONAL        7
#define COORD_EVENT_WEATHER_VOLCANIC_ASH        8
#define COORD_EVENT_WEATHER_SANDSTORM           9
#define COORD_EVENT_WEATHER_SHADE               10
#define COORD_EVENT_WEATHER_DROUGHT             11
#define COORD_EVENT_WEATHER_ROUTE119_CYCLE      20
#define COORD_EVENT_WEATHER_ROUTE123_CYCLE      21

// These are the "abnormal weather events" that are used
// to find Kyogre and Groudon.

// Groudon/Terra Cave locations
#define TERRA_CAVE_LOCATIONS_START          1
#define ABNORMAL_WEATHER_ROUTE_114_NORTH    (TERRA_CAVE_LOCATIONS_START + 0)
#define ABNORMAL_WEATHER_ROUTE_114_SOUTH    (TERRA_CAVE_LOCATIONS_START + 1)
#define ABNORMAL_WEATHER_ROUTE_115_WEST     (TERRA_CAVE_LOCATIONS_START + 2)
#define ABNORMAL_WEATHER_ROUTE_115_EAST     (TERRA_CAVE_LOCATIONS_START + 3)
#define ABNORMAL_WEATHER_ROUTE_116_NORTH    (TERRA_CAVE_LOCATIONS_START + 4)
#define ABNORMAL_WEATHER_ROUTE_116_SOUTH    (TERRA_CAVE_LOCATIONS_START + 5)
#define ABNORMAL_WEATHER_ROUTE_118_EAST     (TERRA_CAVE_LOCATIONS_START + 6)
#define ABNORMAL_WEATHER_ROUTE_118_WEST     (TERRA_CAVE_LOCATIONS_START + 7)
#define TERRA_CAVE_LOCATIONS                8

// Kyogre/Marina Cave locations
#define MARINE_CAVE_LOCATIONS_START         (TERRA_CAVE_LOCATIONS_START + TERRA_CAVE_LOCATIONS)
#define ABNORMAL_WEATHER_ROUTE_105_NORTH    (MARINE_CAVE_LOCATIONS_START + 0)
#define ABNORMAL_WEATHER_ROUTE_105_SOUTH    (MARINE_CAVE_LOCATIONS_START + 1)
#define ABNORMAL_WEATHER_ROUTE_125_WEST     (MARINE_CAVE_LOCATIONS_START + 2)
#define ABNORMAL_WEATHER_ROUTE_125_EAST     (MARINE_CAVE_LOCATIONS_START + 3)
#define ABNORMAL_WEATHER_ROUTE_127_NORTH    (MARINE_CAVE_LOCATIONS_START + 4)
#define ABNORMAL_WEATHER_ROUTE_127_SOUTH    (MARINE_CAVE_LOCATIONS_START + 5)
#define ABNORMAL_WEATHER_ROUTE_129_WEST     (MARINE_CAVE_LOCATIONS_START + 6)
#define ABNORMAL_WEATHER_ROUTE_129_EAST     (MARINE_CAVE_LOCATIONS_START + 7)
#define MARINE_CAVE_LOCATIONS               8

#define ABNORMAL_WEATHER_LOCATIONS  (MARINE_CAVE_LOCATIONS + TERRA_CAVE_LOCATIONS)
#define ABNORMAL_WEATHER_NONE       0

#endif // GUARD_CONSTANTS_WEATHER_H
