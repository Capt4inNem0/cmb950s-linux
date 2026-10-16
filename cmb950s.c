#include <linux/hid.h>
#include <linux/module.h>
#include <linux/mutex.h>

#include "cmb950s.h"

static const struct hid_device_id cmb950s_ids[] = {
	{ HID_USB_DEVICE(0x5131, 0x2007) },
	{ }
};
MODULE_DEVICE_TABLE(hid, cmb950s_ids);

/* ATTRIBUTES */
static ssize_t cpu_temperature_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct cmb950s_drvdata *drvdata = hid_get_drvdata(to_hid_device(dev));
	
	mutex_lock(&drvdata->mutex);
	u8 cpu_temperature = drvdata->report->cpu_temperature;
	mutex_unlock(&drvdata->mutex);

	return sysfs_emit(buf, "%d\n", cpu_temperature);
}

static ssize_t cpu_temperature_store(struct device *dev, struct device_attribute *attr, char const *buf, size_t size){
	struct cmb950s_drvdata *drvdata = hid_get_drvdata(to_hid_device(dev));

	int retval;
	u8 cpu_temperature;
	retval = kstrtou8(buf, 10, &cpu_temperature);
	if (retval) return retval;
	
	mutex_lock(&drvdata->mutex);

	drvdata->report->cpu_temperature = cpu_temperature;
	hid_hw_output_report(to_hid_device(dev), (__u8 *) (drvdata->report), 64);
	
	mutex_unlock(&drvdata->mutex);
	return size;
};
DEVICE_ATTR_RW(cpu_temperature);

static ssize_t cpu_fan_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct cmb950s_drvdata *drvdata = hid_get_drvdata(to_hid_device(dev));
	
	mutex_lock(&drvdata->mutex);
	__be16 cpu_fan = drvdata->report->cpu_fan;
	mutex_unlock(&drvdata->mutex);

	return sysfs_emit(buf, "%d\n", be16_to_cpu(cpu_fan));
}

static ssize_t cpu_fan_store(struct device *dev, struct device_attribute *attr, char const *buf, size_t size){
	struct cmb950s_drvdata *drvdata = hid_get_drvdata(to_hid_device(dev));

	int retval;
	u16 cpu_fan;
	retval = kstrtou16(buf, 10, &cpu_fan);
	if (retval) return retval;
	
	mutex_lock(&drvdata->mutex);

	drvdata->report->cpu_fan = cpu_to_be16(cpu_fan);
	hid_hw_output_report(to_hid_device(dev), (__u8 *) (drvdata->report), 64);
	
	mutex_unlock(&drvdata->mutex);
	return size;
};
DEVICE_ATTR_RW(cpu_fan);

static ssize_t gpu_temperature_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct cmb950s_drvdata *drvdata = hid_get_drvdata(to_hid_device(dev));
	
	mutex_lock(&drvdata->mutex);
	u8 gpu_temperature = drvdata->report->gpu_temperature;
	mutex_unlock(&drvdata->mutex);

	return sysfs_emit(buf, "%d\n", gpu_temperature);
}

static ssize_t gpu_temperature_store(struct device *dev, struct device_attribute *attr, char const *buf, size_t size){
	struct cmb950s_drvdata *drvdata = hid_get_drvdata(to_hid_device(dev));

	int retval;
	u8 gpu_temperature;
	retval = kstrtou8(buf, 10, &gpu_temperature);
	if (retval) return retval;
	
	mutex_lock(&drvdata->mutex);

	drvdata->report->gpu_temperature = gpu_temperature;
	hid_hw_output_report(to_hid_device(dev), (__u8 *) (drvdata->report), 64);
	
	mutex_unlock(&drvdata->mutex);
	return size;
};
DEVICE_ATTR_RW(gpu_temperature);



static struct attribute *cmb950s_attrs[] = {
	&dev_attr_cpu_temperature.attr,
	&dev_attr_cpu_fan.attr,
	&dev_attr_gpu_temperature.attr,
	NULL,
};
ATTRIBUTE_GROUPS(cmb950s);

/* END ATTRIBUTES */

static int cmb950s_probe(struct hid_device *hdev, const struct hid_device_id *id) {
	dev_info(&hdev->dev, "cmb950s device vid=%04x pid=%04x\n", id->vendor, id->product);
	int ret;
	ret = hid_parse(hdev);
	if (ret) return ret;

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	
	struct cmb950s_drvdata *drvdata = devm_kzalloc(&hdev->dev, sizeof(struct cmb950s_drvdata), GFP_KERNEL);
	struct cmb950s_report *buf = devm_kzalloc(&hdev->dev, sizeof(struct cmb950s_report), GFP_KERNEL);
	
	drvdata->report = buf;
	mutex_init(&drvdata->mutex);

	buf->report_id 	= 0x00;
	buf->command	= 0x40;
	buf->cpu_fan		= cpu_to_be16(67);
	buf->gpu_temperature 	= 67;

	hid_set_drvdata(hdev, drvdata);

	return ret;
}

static void cmb950s_remove(struct hid_device *hdev) {
	hid_hw_stop(hdev);
}

static struct hid_driver cmb950s_driver = {
        .name        	= "cmb950s",
        .probe       	= cmb950s_probe,
	.remove		= cmb950s_remove,
	.driver.dev_groups = cmb950s_groups,
        .id_table    	= cmb950s_ids,
};

module_hid_driver(cmb950s_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gastón Collo <gaston.collo@mi.unc.edu.ar>");
MODULE_DESCRIPTION("Controller for CMB950S display");
