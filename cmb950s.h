#include <linux/hid.h>

struct cmb950s_report {
	u8	report_id;
	u8	command;
	u8 	cpu_temperature;// C 
	u8 	cpu_load;	// %
	__be16 	cpu_frequency; 	// Mhz
	__be16 	cpu_fan;	// RPM
	__be16 	cpu_power;	// Watts
	u8 	gpu_temperature;// C
	u8 	gpu_load;	// %
	__be16 	gpu_frequency;	// Mhz
	__be16 	gpu_fan;	// RPM
	__be16 	gpu_power;	// Watts
	u8 padding[46];		// Padding para llegar a 64 bytes
} __packed; // Es una instruccion del compulador para evitar que agregue paddings entre los valores

/* Driver data */
struct cmb950s_drvdata {
	struct cmb950s_report *report;
	struct mutex mutex;
};

/* Sysfs attributes */
struct cmb950s_sysfs_attributes {
	u8 cpu_temperature;
	u16 cpu_fan;
	u8 gpu_temperature;
};

static_assert(sizeof(struct cmb950s_report) == 64);
