// SPDX-License-Identifier: Dual BSD/GPL
/*
 * Copyright (c) 2017, GlobalLogic Ukraine LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the GlobalLogic.
 * 4. Neither the name of the GlobalLogic nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GLOBALLOGIC UKRAINE LLC ``AS IS`` AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL GLOBALLOGIC UKRAINE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_AUTHOR("Vashchishin Ilya <illa123fgh@gmail.com>");
MODULE_DESCRIPTION("Hello, World in Linux Kernel Module");
MODULE_LICENSE("Dual BSD/GPL");

static uint print_count = 1;
module_param(print_count, uint, 0444);
MODULE_PARM_DESC(print_count, "Number of 'Hello, world!' messages to print");

struct hello_entry {
	struct list_head node;
	ktime_t timestamp;
};

static LIST_HEAD(hello_entries);

static void cleanup_list(void)
{
	struct hello_entry *entry, *next;

	list_for_each_entry_safe(entry, next, &hello_entries, node) {
		pr_info("Releasing entry with timestamp: %lld ns\n",
			ktime_to_ns(entry->timestamp));
		list_del(&entry->node);
		kfree(entry);
	}
}

static int __init hello_module_init(void)
{
	int i;
	struct hello_entry *entry;

	if (print_count == 0) {
		pr_warn("print_count is set to 0. Skipping message printing.\n");
		return 0;
	}

	if (print_count > 10) {
		pr_err("print_count exceeds 10. Aborting module load.\n");
		return -EINVAL;
	}

	if (print_count >= 5)
		pr_warn("print_count is %u. Consider lowering this value.\n",
			print_count);

	for (i = 0; i < print_count; i++) {
		entry = kmalloc(sizeof(*entry), GFP_KERNEL);
		if (!entry) {
			cleanup_list();
			return -ENOMEM;
		}

		entry->timestamp = ktime_get();
		list_add_tail(&entry->node, &hello_entries);
		pr_info("Hello, world! (%d/%u)\n", i + 1, print_count);
	}

	pr_info("Module initialized successfully with %u messages.\n",
		print_count);
	return 0;
}

static void __exit hello_module_exit(void)
{
	pr_info("Cleaning up and unloading module...\n");
	cleanup_list();
	pr_info("Module unloaded cleanly.\n");
}

module_init(hello_module_init);
module_exit(hello_module_exit);