enum UnityType
{
    FRIENDLY,
    HOSTILE
};

struct UnitEntity : Entity
{
    UnityType type = UnityType::HOSTILE;
    EntityRef<Entity> overall_target{};


    // float avoid_factor = 30;
    float protection_distance = 40;

    float move_factor = 35;

    float enemy_detection_range = 100;
    float attack_range = 25;

    float attack_merge_range = 5;
    u32 damage = 3;

    u32 health = 10;
    float attack_speed = 1;
    float attack_cooldown = 0;

    void OnRender() override
    {
        DrawSprite(-20, -20, 40, 40, BLUE);
    }

    void Update() override
    {
        if(health == 0) {
            TraceLog(LOG_INFO, "Death");
            DeleteEntity(this);
            return;
        }
        Entity::Update();

        if (attack_cooldown <= 0) return;
        attack_cooldown -= GetFrameTime();
    }

    void Damage(u32 damage_received)
    {
        if (health <= damage_received) {
            health = 0;
            return;
        }
        health -= damage_received;
    }
};


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
        float close_dx = 0;
        float close_dy = 0;


        float avoid_factor = current_target->move_factor;
        float move_factor = current_target->move_factor;
        float enemy_detection_range = current_target->enemy_detection_range;
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
            if (other_target->type != current_target->type) {

                float distanceSqr = Vector3LengthSqr(delta);
                if (distanceSqr > (enemy_detection_range * enemy_detection_range)) {
                    other_target = (UnitEntity *) *other_target->next;
                    continue;
                }
                if (closest_enemy && distanceSqr > closest_enemy_dist) {
                    other_target = (UnitEntity *) *other_target->next;
                    continue;
                }
                closest_enemy_dist = distanceSqr;
                closest_enemy = other_target;


                other_target = (UnitEntity *) *other_target->next;
                continue;
            }

            // Add push away from nearby friendly unit

            if (Vector3LengthSqr(delta) > (protection_distance * protection_distance)) {
                other_target = (UnitEntity *) *other_target->next;
                continue;
            }

            close_dx += delta.x;
            close_dy += delta.y;


            other_target = (UnitEntity *) *other_target->next;
        }

        if (closest_enemy) {
            Vector3 delta = Vector3Subtract(closest_enemy->local_position, current_target->local_position);
            float dist = Vector3LengthSqr(delta);

            if (dist < (attack_range * attack_range)) {

                if (current_target->attack_cooldown > 0) return;

                current_target->attack_cooldown = current_target->attack_speed;
                closest_enemy->Damage(current_target->damage);
                TraceLog(LOG_INFO, "Attack %d(%d) -> %d(%d) for %d. Remaining %d", current_target->type, current_target->id, closest_enemy->type, closest_enemy->id, current_target->damage, closest_enemy->health);
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
};
