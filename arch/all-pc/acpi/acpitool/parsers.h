struct Parser
{
    unsigned int signature;
    char *name;
    void (*parser)();
};

#define BUFFER_SIZE 256

extern char buf[BUFFER_SIZE];
extern APTR ACPIBase;

#define ACPI ((struct ACPIBase *)ACPIBase)

typedef void (*out_func)(const char *);

void header_parser(struct ACPI_TABLE_DEF_HEADER *table, void (*cb)(const char *));

const struct Parser *FindParser(unsigned int signature);
