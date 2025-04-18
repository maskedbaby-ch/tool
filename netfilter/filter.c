/*
 * SO2 - Networking Lab (#10)
 *
 * Exercise #1, #2: simple netfilter module
 *
 * Code skeleton.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/atomic.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include "filter.h"

MODULE_DESCRIPTION("Simple netfilter module");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define LOG_LEVEL		KERN_ALERT
#define MY_DEVICE		"filter"

#define NIPQUAD_FMT "%u.%u.%u.%u"
#define NIPQUAD(addr) ((unsigned char *)&addr)[3], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[0]
#define NIPQUAD2(addr) ((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]
static struct cdev my_cdev;
static atomic_t ioctl_set;
static unsigned int ioctl_set_addr;



static unsigned int my_nf_hookfn(void *priv,
              struct sk_buff *skb,
              const struct nf_hook_state *state)
{

	struct iphdr *iph = ip_hdr(skb);
	if (ioctl_set_addr == iph->daddr){
	//if (ioctl_set_addr == iph->saddr){
		if (iph->protocol == IPPROTO_ICMP){
			struct ethhdr *eth_hdr = skb_eth_hdr(skb);
			printk("dst:%pM src:%pM",eth_hdr->h_dest, eth_hdr->h_source);
			print_hex_dump(KERN_DEBUG, "my_dev_recv before pull: ", DUMP_PREFIX_OFFSET,
                   	16, 1, skb->data, skb->len, false);
			skb_pull(skb, sizeof(struct ethhdr));
			printk("IP address is %pI4\n", &iph->saddr);
			print_hex_dump(KERN_DEBUG, "my_dev_recv after pull: ", DUMP_PREFIX_OFFSET,
                   	16, 1, skb->data, skb->len, false);
			struct ethhdr *mydata = skb_push(skb,sizeof(struct ethhdr));
			memcpy(mydata,eth_hdr,sizeof(struct ethhdr));
		//	mydata->h_dest = eth_hdr->h_dest;
		//	mydata->h_source = eth_hdr->h_source;
		//	mydata->h_proto = eth_hdr->h_proto;
			print_hex_dump(KERN_DEBUG, "my_dev_recv after push: ", DUMP_PREFIX_OFFSET,
                   	16, 1, skb->data, skb->len, false);
		}
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops my_nfho = {
      .hook        = my_nf_hookfn,
      //.hooknum     = NF_INET_PRE_ROUTING,
      .hooknum     = NF_INET_POST_ROUTING,
      .pf          = PF_INET,
      .priority    = NF_IP_PRI_FIRST
};
static int my_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
	return 0;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int v = 0;
	switch (cmd) {
	case MY_IOCTL_FILTER_ADDRESS:
		copy_from_user(&v, (int*)arg, sizeof(int));
		ioctl_set_addr = v;
	//	printk("%u",v);
	//	inet_ntop(AF_INET, &arg, buf, 16);
		printk(NIPQUAD_FMT, NIPQUAD2(v));
		/* TODO 2: set filter address from arg */
		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.unlocked_ioctl = my_ioctl
};

/* TODO 1: define netfilter hook operations structure */

int __init my_hook_init(void)
{
	int err;

	/* register filter device */
	err = register_chrdev_region(MKDEV(MY_MAJOR, 0), 1, MY_DEVICE);
	if (err != 0)
		return err;

	atomic_set(&ioctl_set, 0);
	ioctl_set_addr = 0;

	/* init & add device */
	cdev_init(&my_cdev, &my_fops);
	cdev_add(&my_cdev, MKDEV(MY_MAJOR, 0), 1);

	/* TODO 1: register netfilter hook */
	nf_register_net_hook(&init_net, &my_nfho);

	return 0;

out:
	/* cleanup */
	cdev_del(&my_cdev);
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);

	return err;
}

void __exit my_hook_exit(void)
{
	/* TODO 1: unregister hook */
	nf_unregister_net_hook(&init_net, &my_nfho);

	/* cleanup device */
	cdev_del(&my_cdev);
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);
}

module_init(my_hook_init);
module_exit(my_hook_exit);
