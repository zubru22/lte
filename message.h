struct RA_RNTI{

};
struct C_RNTI{
};

struct message{
    enum type_of_message{RA_RNTI, C_RNTI};
    union {
    };
};