enum UnitType
{
    UnitType_LIGHT,
    UnitType_ARCHER,
    UnitType_TANK,
    UnitType_MEDIC,
};


struct UnitData {

    UnitType type;
    u32 amount;
    bool hostile;

//    float protection_distance;
//
//    float move_factor;
//
//    float enemy_detection_range;
//    float attack_range;
//    float projectile_speed;
//
//    float prioritized_range;
//
//    float attack_merge_range;
//    u32 damage;
//
//    u32 health;
//    u32 max_health;
//    float attack_speed;
};
