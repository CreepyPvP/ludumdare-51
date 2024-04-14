enum UnityType
{
    FRIENDLY,
    HOSTILE
};

struct UnitEntity : Entity
{
    UnityType type = UnityType::HOSTILE;
    EntityRef<Entity> overall_target{};

    void Render() override {
        DrawRectangle(-50, -50, 50, 50, BLUE);
    }
};


struct UnitManagementEntity : Entity
{

    float avoid_factor = 0.1;
    float move_factor = 10;
    float protection_distance = 40;
    float attack_range = 5;
    float attack_merge_range = 5;


    void Update() override
    {
        Entity::Update();
        UnitEntity *current_target = (UnitEntity *) *child;

        while (current_target) {


            current_target->Update();

            UnitEntity *closest_enemy;
            float closest_enemy_dist = -1;
            float close_dx = 0;
            float close_dy = 0;


            UnitEntity *other_target = (UnitEntity *) *child;
            while (other_target) {
                if (other_target->id == current_target->id) {
                    other_target = (UnitEntity *) *other_target->next;
                    continue;
                }


                Vector3 delta = Vector3Subtract(current_target->local_position, other_target->local_position);
                if (other_target->type != current_target->type) {

                    float distanceSqr = Vector3LengthSqr(delta);
                    if(closest_enemy && distanceSqr > closest_enemy_dist) {
                        return;
                    }
                    closest_enemy_dist = distanceSqr;
                    closest_enemy = other_target;


                    other_target = (UnitEntity *) *other_target->next;
                    continue;
                }

                // Add push away from nearby friendly unit

                if (Vector3LengthSqr(delta) > protection_distance) continue;

                close_dx += delta.x;
                close_dy += delta.y;


                other_target = (UnitEntity *) *other_target->next;
            }

            if(closest_enemy) {
                Vector3 delta = Vector3Subtract(current_target->local_position, closest_enemy->local_position);
                float dist = Vector3LengthSqr(delta);

                if(dist < attack_range) {
                    TraceLog(LOG_INFO, "Attack");
                    return;
                }

                current_target->local_position.x += delta.x * move_factor * GetFrameTime();
                current_target->local_position.y += delta.y * move_factor * GetFrameTime();
            } else if(*current_target->overall_target) {
                Vector3 delta = Vector3Subtract(current_target->local_position, current_target->overall_target->local_position);
                float dist = Vector3LengthSqr(delta);

                if(dist < attack_merge_range) {
                    TraceLog(LOG_INFO, "Merge into core");
                    return;
                }

                current_target->local_position.x += delta.x * move_factor * GetFrameTime();
                current_target->local_position.y += delta.y * move_factor * GetFrameTime();
            }

            current_target->local_position.x += close_dx * avoid_factor * GetFrameTime();
            current_target->local_position.y += close_dy * avoid_factor * GetFrameTime();
            current_target = (UnitEntity *) *current_target->next;
        }
    }
};
