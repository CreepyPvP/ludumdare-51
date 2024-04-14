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
    float protection_distance = 50;

    float move_factor = 30;

    float enemy_detection_range = 100;
    float attack_range = 50;

    float attack_merge_range = 5;

    void OnRender() override
    {
        DrawSprite(-50, -50, 100, 100, BLUE, 0);
    }
};


struct UnitManagementEntity : Entity
{


    void Update() override
    {
        UnitEntity *current_target = (UnitEntity *) *child;

        while (current_target) {

            current_target->Update();
            HandleEntityMovement(current_target);
            current_target = (UnitEntity *) *current_target->next;
        }
    }

    void HandleEntityMovement(UnitEntity *current_target) {
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
                if(distanceSqr > (enemy_detection_range * enemy_detection_range)) {
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
                TraceLog(LOG_INFO, "Attack");
                return;
            }

            Vector3  normDelta = Vector3Normalize(delta);

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

            Vector3  normDelta = Vector3Normalize(delta);

            current_target->local_position.x += normDelta.x * move_factor * GetFrameTime();
            current_target->local_position.y += normDelta.y * move_factor * GetFrameTime();
        }

        Vector2 deltaNorm = Vector2Normalize({close_dx, close_dy});
        current_target->local_position.x += deltaNorm.x * avoid_factor * GetFrameTime();
        current_target->local_position.y += deltaNorm.y * avoid_factor * GetFrameTime();
    }
};
