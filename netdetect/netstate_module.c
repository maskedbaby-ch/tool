#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define DEVICE_NAME "netstate"

static DECLARE_WAIT_QUEUE_HEAD(netstate_waitq);
static atomic_t data_ready = ATOMIC_INIT(0);
static char netstate_data[64] = {0};

static int netstate_notifier(struct notifier_block *nb, 
                           unsigned long state, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    
    if (!dev) {
        pr_info("No net device in notifier\n");
        return NOTIFY_DONE;
    }

    switch (state) {
    case NETDEV_UP:
        snprintf(netstate_data, sizeof(netstate_data), "%s:UP", dev->name);
        pr_info("Network %s is UP\n", dev->name);  // 添加内核日志
        atomic_set(&data_ready, 1);
        wake_up_interruptible(&netstate_waitq);
        break;
    case NETDEV_DOWN:
        snprintf(netstate_data, sizeof(netstate_data), "%s:DOWN", dev->name);
        pr_info("Network %s is DOWN\n", dev->name);  // 添加内核日志
        atomic_set(&data_ready, 1);
        wake_up_interruptible(&netstate_waitq);
        break;
    }
    return NOTIFY_DONE;
}

static struct notifier_block netstate_nb = {
    .notifier_call = netstate_notifier,
};

static ssize_t netstate_read(struct file *file, char __user *buf,
                           size_t count, loff_t *ppos)
{
    int ret;

    if (wait_event_interruptible(netstate_waitq, atomic_read(&data_ready)))
        return -ERESTARTSYS;

    if (copy_to_user(buf, netstate_data, strlen(netstate_data) + 1))
        return -EFAULT;

    atomic_set(&data_ready, 0);
    return strlen(netstate_data) + 1;
}

static const struct file_operations netstate_fops = {
    .owner = THIS_MODULE,
    .read = netstate_read,
};

static struct miscdevice netstate_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &netstate_fops,
};

static int __init netstate_init(void)
{
    int ret;
    
    ret = misc_register(&netstate_dev);
    if (ret) {
        pr_err("Failed to register misc device\n");
        return ret;
    }

    register_netdevice_notifier(&netstate_nb);
    pr_info("NetState module loaded\n");
    return 0;
}

static void __exit netstate_exit(void)
{
    unregister_netdevice_notifier(&netstate_nb);
    misc_deregister(&netstate_dev);
    pr_info("NetState module unloaded\n");
}

module_init(netstate_init);
module_exit(netstate_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Da");
MODULE_DESCRIPTION("Network state monitoring module");

