enum UnitTeam
{
    HOSTILE,
    FRIENDLY,
};

enum UnitAttackType
{
    MELEE,
    RANGED
};

enum TargetingType
{
    ENEMY,
    TEAM
};

struct TesseractEntity : Entity
{

    u32 damage_sfx_counter;
    u32 health;

    void OnRender() override
    {
        DrawSprite(0, 0, 40 * 3, 40 * 3, ORANGE, AppearanceType::TESSERACT, 0);
        const char *text = TextFormat("%d", health);
        DrawText(text, -MeasureText(text, 20) / 2, -10, 20, RED);

    }

    void Damage(u32 damage)
    {
        PlaySound(state->tesseract_damaged_sound[damage_sfx_counter]);

        damage_sfx_counter = (damage_sfx_counter + 1) % 3;

        if (health <= damage) {
            health = 0;
            return;
        }
        health -= damage;
    }

};

struct UnitEntity : Entity
{

    UnitTeam team;
    UnitAttackType attack_type;
    TargetingType targeting_type;
    AppearanceType appearance;

    EntityRef<TesseractEntity> overall_target{};
    EntityRef<Entity> projectile_container{};


    // float avoid_factor = 30;
    float protection_distance;

    float move_factor;

    float enemy_detection_range;
    float attack_range;
    float projectile_speed;

    // Attacks lowest HP enemy in prio range
    bool prioritize_execute;
    float prioritized_range;

    float attack_merge_range;
    u32 damage;

    u32 health;
    u32 max_health;
    float attack_speed;
    float attack_cooldown;

    bool is_fake;

    inline Color GetColor() const
    {
        Color color = RED;
        if (team == FRIENDLY) {
            if (appearance == TANK) {
                color = BLUE;
            } else if (appearance == ARCHER) {
                color = PURPLE;
            } else if (appearance == MEDIC) {
                color = GREEN;
            } else if (appearance == LIGHT) {
                color = {0, 121, 190, 255};
            }
        }

        return color;
    }


    void OnRender() override
    {
        float dist = team == HOSTILE ? 1 : 0;
        DrawSprite(0, 0, 40, 40, GetColor(), appearance, dist);
    }

    void Update() override
    {
        if (health == 0) {
            TraceLog(LOG_INFO, "Death");
            DeleteEntity(this);
            return;
        }
        Entity::Update();

        if (attack_cooldown <= 0) return;
        attack_cooldown -= GetFrameTime();
    }

    void OnEnable() override
    {
        Entity::OnEnable();

        if (appearance >= UNIT_TYPE_COUNT || team != FRIENDLY || is_fake) {
            return;
        }

        state->alive_units[appearance]++;
    }

    void OnDisable() override
    {
        Entity::OnDisable();

        if (appearance >= UNIT_TYPE_COUNT || team != FRIENDLY || is_fake) {
            return;
        }

        state->alive_units[appearance]--;
    }

    void Damage(u32 damage_received)
    {
        if (health <= damage_received) {
            health = 0;
            return;
        }
        health -= damage_received;
    }

    void Heal(u32 damage_received)
    {
        health += damage_received;
        if (health <= max_health) return;
        health = max_health;
    }

    void TryAttack(UnitEntity *target_unit);

    float GetHealthPercentage() const
    {
        return (float) health / (float) max_health;
    }

    void OnDestroy() override
    {
        if (!is_fake && team == UnitTeam::HOSTILE) {
            state->stats.enemies_killed += 1;
        }
        Entity::OnDestroy();

    }
};

struct ProjectileEntity : Entity
{
    EntityRef<UnitEntity> target{};
    float speed = 100;
    TargetingType targeting_type;
    u32 damage = 3;
    Color color = BLUE;

    float hitRange = 5;

    void Update() override
    {
        UnitEntity *target_ref = *target;
        if (!target_ref) {
            DeleteEntity(this);
            return;
        }

        Vector3 delta = Vector3Subtract(target_ref->local_position, local_position);
        float combinedDistance = hitRange + target_ref->protection_distance / 2.0f;
        if (Vector3LengthSqr(delta) < combinedDistance * combinedDistance) {
            HitTarget();
            return;
        }

        delta = Vector3Normalize(delta);

        local_position.x += delta.x * speed * GetFrameTime();
        local_position.y += delta.y * speed * GetFrameTime();

    }

    void OnRender() override
    {
        DrawSprite(0, 0, 10, 10, color, LIGHT, 0);
    }

    void HitTarget()
    {
        if (targeting_type == ENEMY) {
            target->Damage(damage);
        } else {
            target->Heal(damage);
        }
        DeleteEntity(this);
    }

};


void UnitEntity::TryAttack(UnitEntity *target_unit)
{
    if (targeting_type == TargetingType::TEAM && target_unit->GetHealthPercentage() >= 1) {
        return;
    }

    if (this->attack_cooldown > 0) return;

    if (attack_type == UnitAttackType::MELEE) {
        this->attack_cooldown = this->attack_speed;
        if (targeting_type == TargetingType::ENEMY) {
            target_unit->Damage(this->damage);
            TraceLog(LOG_INFO, "Attack %d(%d) -> %d(%d) for %d. Remaining %d", this->team, this->id, target_unit->team,
                     target_unit->id, this->damage, target_unit->health);
        } else {
            target_unit->Heal(this->damage);
            TraceLog(LOG_INFO, "Heal %d(%d) -> %d(%d) for %d. Remaining %d", this->team, this->id, target_unit->team,
                     target_unit->id, this->damage, target_unit->health);
        }
    } else if (attack_type == UnitAttackType::RANGED) {
        this->attack_cooldown = this->attack_speed;

        ProjectileEntity *projectile = AllocateEntity<ProjectileEntity>();
        projectile->local_position = local_position;
        projectile->targeting_type = targeting_type;
        projectile->damage = damage;
        projectile->color = GetColor();
        projectile->speed = projectile_speed;
        projectile->target = MakeRef<UnitEntity>(target_unit);

        TraceLog(LOG_INFO, "Shoot %d(%d) -> %d(%d) for %d.", this->team, this->id, target_unit->team,
                 target_unit->id, this->damage);

        projectile_container->PushChild(projectile);
    }
}

struct UnitManagementEntity : Entity
{


    void Update() override
    {
        Entity::Update();

        UnitEntity *current_target = (UnitEntity *) *child;

        while (current_target) {

            HandleEntityMovement(current_target);
            current_target = (UnitEntity *) *current_target->next;
        }
    }

    void HandleEntityMovement(UnitEntity *current_target)
    {
        UnitEntity *closest_enemy = nullptr;
        float closest_enemy_dist = -1;
        UnitEntity *priority_enemy = nullptr;
        float priority_enemy_dist = -1;

        // In percentage
        float priority_enemy_health = 1;

        float close_dx = 0;
        float close_dy = 0;


        float avoid_factor = current_target->move_factor;
        float move_factor = current_target->move_factor;
        float enemy_detection_range = current_target->enemy_detection_range;
        float prioritized_range = current_target->prioritized_range;
        float protection_distance = current_target->protection_distance;
        float attack_range = current_target->attack_range;
        float attack_merge_range = current_target->attack_merge_range;


        UnitEntity *other_target = (UnitEntity *) *child;
        while (other_target) {
            if (other_target->id == current_target->id) {
                other_target = (UnitEntity *) *other_target->next;
                continue;
            }


            Vector3 delta = Vector3Subtract(current_target->local_position, other_target->local_position);
            // Select attack target follow
            if (current_target->targeting_type == TargetingType::ENEMY == other_target->team != current_target->team) {


                float distanceSqr = Vector3LengthSqr(delta);

                if (current_target->targeting_type != TargetingType::TEAM ||
                    current_target->appearance != other_target->appearance) {
                    HandleNormalAttackCheck(distanceSqr, enemy_detection_range, closest_enemy_dist, closest_enemy,
                                            other_target);
                }

                if (current_target->prioritize_execute) {
                    HandlePriorityAttackCheck(distanceSqr, prioritized_range, priority_enemy_dist,
                                              priority_enemy_health, priority_enemy, other_target);
                }

                other_target = (UnitEntity *) *other_target->next;
                continue;
            }

            // Add push away from nearby friendly unit
            float dist = Vector3LengthSqr(delta);
            if (dist > (protection_distance * protection_distance)) {
                other_target = (UnitEntity *) *other_target->next;
                continue;
            }

            if (dist < 0.2f) {
                switch (GetRandomValue(0, 3)) {
                    case 0:
                        delta = Vector3{1, 0, 0};
                        break;
                    case 1:
                        delta = Vector3{0, 1, 0};
                        break;
                    case 2:
                        delta = Vector3{-1, 0, 0};
                        break;
                    case3:
                        delta = Vector3{0, -1, 0};
                        break;
                }
            }

            close_dx += delta.x;
            close_dy += delta.y;


            other_target = (UnitEntity *) *other_target->next;
        }

        if (priority_enemy) {
            closest_enemy = priority_enemy;
        }


        if (closest_enemy) {
            Vector3 delta = Vector3Subtract(closest_enemy->local_position, current_target->local_position);
            float dist = Vector3LengthSqr(delta);

            if (dist < (attack_range * attack_range)) {

                current_target->TryAttack(closest_enemy);
                return;
            }

            Vector3 normDelta = Vector3Normalize(delta);

            current_target->local_position.x += normDelta.x * move_factor * GetFrameTime();
            current_target->local_position.y += normDelta.y * move_factor * GetFrameTime();
        } else if (*current_target->overall_target) {
            Vector3 delta = Vector3Subtract(current_target->overall_target->local_position,
                                            current_target->local_position);
            float dist = Vector3LengthSqr(delta);

            if (dist < (attack_merge_range * attack_merge_range)) {
                TraceLog(LOG_INFO, "Merge into core");
                current_target->overall_target->Damage(current_target->damage);
                DeleteEntity(current_target);
                return;
            }

            Vector3 normDelta = Vector3Normalize(delta);

            current_target->local_position.x += normDelta.x * move_factor * GetFrameTime();
            current_target->local_position.y += normDelta.y * move_factor * GetFrameTime();
        }

        Vector2 deltaNorm = Vector2Normalize({close_dx, close_dy});
        current_target->local_position.x += deltaNorm.x * avoid_factor * GetFrameTime();
        current_target->local_position.y += deltaNorm.y * avoid_factor * GetFrameTime();
    }

    static void HandlePriorityAttackCheck(float distanceSqr, float prioritized_range, float &priority_enemy_dist,
                                          float &priority_enemy_health, UnitEntity *&priority_enemy,
                                          UnitEntity *other_target)
    {
        if (distanceSqr > (prioritized_range * prioritized_range)) {
            return;
        }
        float health_perc = other_target->GetHealthPercentage();

        if (!priority_enemy) {
            if (health_perc >= 1) return;
            priority_enemy = other_target;
            priority_enemy_health = health_perc;
            priority_enemy_dist = distanceSqr;
            return;
        }

        if (health_perc > priority_enemy_health) return;
        priority_enemy = other_target;
        priority_enemy_health = health_perc;
        priority_enemy_dist = distanceSqr;
    }

    static void HandleNormalAttackCheck(float distanceSqr, float enemy_detection_range, float &closest_enemy_dist,
                                        UnitEntity *&closest_enemy, UnitEntity *other_target)
    {
        if (distanceSqr > (enemy_detection_range * enemy_detection_range)) {
            return;
        }
        if (closest_enemy && distanceSqr > closest_enemy_dist) {
            return;
        }
        closest_enemy_dist = distanceSqr;
        closest_enemy = other_target;
    }
};


void ConfigureFriendly(UnitEntity *unit)
{
    unit->team = UnitTeam::FRIENDLY;

    unit->enemy_detection_range = 999999;
    unit->move_factor = 100;

    unit->protection_distance = 40;
}

void ConfigureHostile(UnitEntity *unit, TesseractEntity *tesseract)
{
    unit->team = UnitTeam::HOSTILE;
    unit->overall_target = MakeRef<TesseractEntity>(tesseract);

    unit->enemy_detection_range = 100;
    unit->move_factor = 35;
    unit->attack_merge_range = 5;

    unit->protection_distance = 40;
}

void ConfigureTank(UnitEntity *unit)
{
    unit->appearance = AppearanceType::TANK;
    unit->attack_range = 25;
    unit->damage = 2;

    unit->health = 30;
    unit->max_health = 30;

    unit->attack_speed = 1;
}

void ConfigureLight(UnitEntity *unit)
{
    unit->attack_range = 25;
    unit->damage = 5;

    unit->health = 20;
    unit->max_health = 20;

    unit->attack_speed = 0.8;
}

void ConfigureArcher(UnitEntity *unit)
{
    unit->attack_type = UnitAttackType::RANGED;
    unit->appearance = AppearanceType::ARCHER;
    unit->attack_range = 100;
    unit->damage = 3;

    unit->health = 10;
    unit->max_health = 10;

    unit->attack_speed = 1;
    unit->projectile_speed = 150;
}

void ConfigureMedic(UnitEntity *unit)
{
    unit->attack_type = UnitAttackType::RANGED;
    unit->appearance = AppearanceType::MEDIC;
    unit->attack_range = 100;
    unit->damage = 3;

    unit->health = 10;
    unit->max_health = 10;

    unit->attack_speed = 1;
    unit->projectile_speed = 150;

    unit->targeting_type = TargetingType::TEAM;
    unit->prioritize_execute = true;
    unit->prioritized_range = unit->attack_range * 2;
}

void ConfigureFromData(UnitEntity *unit, UnitData &data, TesseractEntity *tesseract)
{
    if (data.hostile) {
        ConfigureHostile(unit, tesseract);
    } else {
        ConfigureFriendly(unit);
    }

    switch (data.type) {
        case UnitType_LIGHT:
            ConfigureLight(unit);
            break;
        case UnitType_ARCHER:
            ConfigureArcher(unit);
            break;
        case UnitType_TANK:
            ConfigureTank(unit);
            break;
        case UnitType_MEDIC:
            ConfigureMedic(unit);
            break;
    }
}

struct PentagramEntity : Entity
{

    float spawn_timer;
    UnitData data;

    Sound sound;

    EntityRef<Entity> unit_container_ref{};
    EntityRef<Entity> projectile_container_ref{};
    EntityRef<TesseractEntity> tesseract_ref{};

    bool finished;

    void OnEnable() override
    {
        Entity::OnEnable();
        sound = LoadSoundAlias(state->unit_summoned_sound[data.type]);
    }

    void OnDisable() override
    {
        UnloadSoundAlias(sound);
    }

    void SpawnUnits()
    {
        finished = true;
        PlaySound(sound);
        spawn_timer = 0.5f;
        for (int i = 0; i < data.amount; ++i) {
            UnitEntity *unit = AllocateEntity<UnitEntity>();

            unit->local_position = local_position;

            unit->projectile_container = projectile_container_ref;
            ConfigureFromData(unit, data, *tesseract_ref);

            unit_container_ref->PushChild(unit);
        }
    }

    void Update() override
    {
        Entity::Update();

        spawn_timer -= GetFrameTime();
        if (spawn_timer < 0) {
            if (!finished) {
                SpawnUnits();
            } else {

                DeleteEntity(this);
            }
        }
    }

    void OnRender() override
    {
        DrawSprite(0, 0, 40 * 3, 40 * 3, RED, PENTAGRAM, 0);
    }

};

struct PentagramEntitySpawner : Entity
{

    EntityRef<Entity> unit_container_ref{};
    EntityRef<Entity> projectile_container_ref{};
    EntityRef<TesseractEntity> tesseract_ref{};

    void Summon(Vector3 position, UnitData data)
    {
        PentagramEntity *penta = AllocateEntity<PentagramEntity>();
        penta->data = data;
        penta->spawn_timer = 3;
        penta->local_position = position;
        penta->unit_container_ref = unit_container_ref;
        penta->projectile_container_ref = projectile_container_ref;
        penta->tesseract_ref = tesseract_ref;
        PushChild(penta);
    }


    void OnCreate() override
    {
        Entity::OnCreate();
    }

    void OnDestroy() override
    {
        Entity::OnDestroy();
    }
};

