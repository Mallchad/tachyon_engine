
class global_database
{
public:
    static global_database primary_database;

    global_database& get_database();
    bool kill_program = false;
};

