/**
 * main.c
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 * Copyright (c) 2015 Jaegeuk Kim <jaegeuk@kernel.org>
 *  : implement defrag.f2fs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck.h"
#include <libgen.h>

#include <sys/time.h>
#include <time.h>

struct f2fs_fsck gfsck;

void fsck_usage()
{
	MSG(0, "\nUsage: fsck.f2fs [options] device\n");
	MSG(0, "[options]:\n");
	MSG(0, "  -a check/fix potential corruption, reported by f2fs\n");
	MSG(0, "  -d debug level [default:0]\n");
	MSG(0, "  -f check/fix entire partition\n");
	MSG(0, "  -p preen mode [default:0 the same as -a [0|1]]\n");
	MSG(0, "  -t show directory tree [-d -1]\n");
	exit(1);
}

void dump_usage()
{
	MSG(0, "\nUsage: dump.f2fs [options] device\n");
	MSG(0, "[options]:\n");
	MSG(0, "  -d debug level [default:0]\n");
	MSG(0, "  -i inode no (hex)\n");
	MSG(0, "  -s [SIT dump segno from #1~#2 (decimal), for all 0~-1]\n");
	MSG(0, "  -a [SSA dump segno from #1~#2 (decimal), for all 0~-1]\n");
	MSG(0, "  -b blk_addr (in 4KB)\n");

	exit(1);
}

void defrag_usage()
{
	MSG(0, "\nUsage: defrag.f2fs [options] device\n");
	MSG(0, "[options]:\n");
	MSG(0, "  -d debug level [default:0]\n");
	MSG(0, "  -s start block address [default: main_blkaddr]\n");
	MSG(0, "  -l length [default:512 (2MB)]\n");
	MSG(0, "  -t target block address [default: main_blkaddr + 2MB]\n");
	MSG(0, "  -i set direction as shrink [default: expand]\n");
	exit(1);
}

void f2fs_parse_options(int argc, char *argv[])
{
	int option = 0;
	char *prog = basename(argv[0]);

	if (!strcmp("fsck.f2fs", prog) || !strcmp("fsck.f2fs_s", prog)) {
		const char *option_string = "ad:fp:t";

		config.func = FSCK;
		while ((option = getopt(argc, argv, option_string)) != EOF) {
			switch (option) {
			case 'a':
				config.auto_fix = 1;
				MSG(0, "Info: Fix the reported corruption.\n");
				break;
			case 'p':
				/* preen mode has different levels:
				 *  0: default level, the same as -a
				 *  1: check meta
				 */
				config.preen_mode = atoi(optarg);
				if (config.preen_mode < 0)
					config.preen_mode = PREEN_MODE_0;
				else if (config.preen_mode >= PREEN_MODE_MAX)
					config.preen_mode = PREEN_MODE_MAX - 1;
				if (config.preen_mode == PREEN_MODE_0)
					config.auto_fix = 1;
				MSG(0, "Info: Fix the reported corruption in "
					"preen mode %d\n", config.preen_mode);
				break;
			case 'd':
				config.dbg_lv = atoi(optarg);
				MSG(0, "Info: Debug level = %d\n",
							config.dbg_lv);
				break;
			case 'f':
				config.fix_on = 1;
				MSG(0, "Info: Force to fix corruption\n");
				break;
			case 't':
				config.dbg_lv = -1;
				break;
			default:
				MSG(0, "\tError: Unknown option %c\n", option);
				fsck_usage();
				break;
			}
		}
	} else if (!strcmp("dump.f2fs", prog)) {
		const char *option_string = "d:i:s:a:b:";
		static struct dump_option dump_opt = {
			.nid = 3,	/* default root ino */
			.start_sit = -1,
			.end_sit = -1,
			.start_ssa = -1,
			.end_ssa = -1,
			.blk_addr = -1,
		};

		config.func = DUMP;
		while ((option = getopt(argc, argv, option_string)) != EOF) {
			int ret = 0;

			switch (option) {
			case 'd':
				config.dbg_lv = atoi(optarg);
				MSG(0, "Info: Debug level = %d\n",
							config.dbg_lv);
				break;
			case 'i':
				if (strncmp(optarg, "0x", 2))
					ret = sscanf(optarg, "%d",
							&dump_opt.nid);
				else
					ret = sscanf(optarg, "%x",
							&dump_opt.nid);
				break;
			case 's':
				ret = sscanf(optarg, "%d~%d",
							&dump_opt.start_sit,
							&dump_opt.end_sit);
				break;
			case 'a':
				ret = sscanf(optarg, "%d~%d",
							&dump_opt.start_ssa,
							&dump_opt.end_ssa);
				break;
			case 'b':
				if (strncmp(optarg, "0x", 2))
					ret = sscanf(optarg, "%d",
							&dump_opt.blk_addr);
				else
					ret = sscanf(optarg, "%x",
							&dump_opt.blk_addr);
				break;
			default:
				MSG(0, "\tError: Unknown option %c\n", option);
				dump_usage();
				break;
			}
			ASSERT(ret >= 0);
		}

		config.private = &dump_opt;
	} else if (!strcmp("defrag.f2fs", prog)) {
		const char *option_string = "d:s:l:t:i";

		config.func = DEFRAG;
		while ((option = getopt(argc, argv, option_string)) != EOF) {
			int ret = 0;

			switch (option) {
			case 'd':
				config.dbg_lv = atoi(optarg);
				MSG(0, "Info: Debug level = %d\n",
							config.dbg_lv);
				break;
			case 's':
				if (strncmp(optarg, "0x", 2))
					ret = sscanf(optarg, "%"PRIu64"",
							&config.defrag_start);
				else
					ret = sscanf(optarg, "%"PRIx64"",
							&config.defrag_start);
				break;
			case 'l':
				if (strncmp(optarg, "0x", 2))
					ret = sscanf(optarg, "%"PRIu64"",
							&config.defrag_len);
				else
					ret = sscanf(optarg, "%"PRIx64"",
							&config.defrag_len);
				break;
			case 't':
				if (strncmp(optarg, "0x", 2))
					ret = sscanf(optarg, "%"PRIu64"",
							&config.defrag_target);
				else
					ret = sscanf(optarg, "%"PRIx64"",
							&config.defrag_target);
				break;
			case 'i':
				config.defrag_shrink = 1;
				break;
			default:
				MSG(0, "\tError: Unknown option %c\n", option);
				defrag_usage();
				break;
			}
			ASSERT(ret >= 0);
		}
	}

	if ((optind + 1) != argc) {
		MSG(0, "\tError: Device not specified\n");
		if (config.func == FSCK)
			fsck_usage();
		else if (config.func == DUMP)
			dump_usage();
		else if (config.func == DEFRAG)
			defrag_usage();
	}
	config.device_name = argv[optind];
}

static void do_fsck(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	u32 flag = le32_to_cpu(ckpt->ckpt_flags);
	u32 blk_cnt;

	fsck_init(sbi);

	print_cp_state(flag);

	if (!config.fix_on && !config.bug_on) {
		switch (config.preen_mode) {
		case PREEN_MODE_1:
			if (fsck_chk_meta(sbi)) {
				MSG(0, "[FSCK] F2FS metadata   [Fail]");
				MSG(0, "\tError: meta does not match, "
					"force check all\n");
			} else {
				MSG(0, "[FSCK] F2FS metadata   [Ok..]");
				fsck_free(sbi);
				return;
			}

			if (!config.ro)
				config.fix_on = 1;
			break;
		}
	} else {
		/*
		 * we can hit this in 3 situations:
		 *  1. fsck -f, fix_on has already been set to 1 when
		 *     parsing options;
		 *  2. fsck -a && CP_FSCK_FLAG is set, fix_on has already
		 *     been set to 1 when checking CP_FSCK_FLAG;
		 *  3. fsck -p 1 && error is detected, then bug_on is set,
		 *     we set fix_on = 1 here, so that fsck can fix errors
		 *     automatically
		*/
		config.fix_on = 1;
	}

	fsck_chk_orphan_node(sbi);

	/* Traverse all block recursively from root inode */
	blk_cnt = 1;
	fsck_chk_node_blk(sbi, NULL, sbi->root_ino_num, (u8 *)"/",
			F2FS_FT_DIR, TYPE_INODE, &blk_cnt, NULL, NULL);
	fsck_verify(sbi);
	fsck_free(sbi);
}

static void do_dump(struct f2fs_sb_info *sbi)
{
	struct dump_option *opt = (struct dump_option *)config.private;
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	u32 flag = le32_to_cpu(ckpt->ckpt_flags);

	fsck_init(sbi);

	if (opt->end_sit == -1)
		opt->end_sit = SM_I(sbi)->main_segments;
	if (opt->end_ssa == -1)
		opt->end_ssa = SM_I(sbi)->main_segments;
	if (opt->start_sit != -1)
		sit_dump(sbi, opt->start_sit, opt->end_sit);
	if (opt->start_ssa != -1)
		ssa_dump(sbi, opt->start_ssa, opt->end_ssa);
	if (opt->blk_addr != -1) {
		dump_info_from_blkaddr(sbi, opt->blk_addr);
		goto cleanup;
	}

	print_cp_state(flag);

	dump_node(sbi, opt->nid);
cleanup:
	fsck_free(sbi);
}

static int do_defrag(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	if (config.defrag_start > get_sb(block_count))
		goto out_range;
	if (config.defrag_start < SM_I(sbi)->main_blkaddr)
		config.defrag_start = SM_I(sbi)->main_blkaddr;

	if (config.defrag_len == 0)
		config.defrag_len = sbi->blocks_per_seg;

	if (config.defrag_start + config.defrag_len > get_sb(block_count))
		config.defrag_len = get_sb(block_count) - config.defrag_start;

	if (config.defrag_target == 0) {
		config.defrag_target = config.defrag_start - 1;
		if (!config.defrag_shrink)
			config.defrag_target += config.defrag_len + 1;
	}

	if (config.defrag_target < SM_I(sbi)->main_blkaddr ||
			config.defrag_target > get_sb(block_count))
		goto out_range;
	if (config.defrag_target >= config.defrag_start &&
		config.defrag_target < config.defrag_start + config.defrag_len)
		goto out_range;

	if (config.defrag_start > config.defrag_target)
		MSG(0, "Info: Move 0x%"PRIx64" <- [0x%"PRIx64"-0x%"PRIx64"]\n",
				config.defrag_target,
				config.defrag_start,
				config.defrag_start + config.defrag_len - 1);
	else
		MSG(0, "Info: Move [0x%"PRIx64"-0x%"PRIx64"] -> 0x%"PRIx64"\n",
				config.defrag_start,
				config.defrag_start + config.defrag_len - 1,
				config.defrag_target);

	return f2fs_defragment(sbi, config.defrag_start, config.defrag_len,
			config.defrag_target, config.defrag_shrink);
out_range:
	ASSERT_MSG("Out-of-range [0x%"PRIx64" ~ 0x%"PRIx64"] to 0x%"PRIx64"",
				config.defrag_start,
				config.defrag_start + config.defrag_len - 1,
				config.defrag_target);
	return -1;
}

int main(int argc, char **argv)
{
	struct f2fs_sb_info *sbi;
	int ret = 0;

	struct timeval t_start;
	struct timeval t_end;
	long start_ms = 0;
	long end_ms = 0;
	long cost_ms = 0;

	/* get start time */
	gettimeofday(&t_start, NULL);
	start_ms = ((long)t_start.tv_sec)*1000 + (long)t_start.tv_usec/1000;
	printf("\nStart time: %ld ms\n", start_ms);

	f2fs_init_configuration(&config);

	f2fs_parse_options(argc, argv);

	if (f2fs_dev_is_umounted(&config) < 0) {
		if (!config.ro || config.func == DEFRAG) {
			MSG(0, "\tError: Not available on mounted device!\n");
			return -1;
		}

		/* allow ro-mounted partition */
		MSG(0, "Info: Check FS only due to RO\n");
		config.fix_on = 0;
		config.auto_fix = 0;
	}

	/* Get device */
	if (f2fs_get_device_info(&config) < 0)
		return -1;
fsck_again:
	memset(&gfsck, 0, sizeof(gfsck));
	gfsck.sbi.fsck = &gfsck;
	sbi = &gfsck.sbi;

	ret = f2fs_do_mount(sbi);
	if (ret == 1) {
		if (sbi->ckpt)
			free(sbi->ckpt);
		if (sbi->raw_super)
			free(sbi->raw_super);
		goto out;
	}
	else if (ret != 0)
		goto out_err;

	switch (config.func) {
	case FSCK:
		do_fsck(sbi);
		break;
	case DUMP:
		do_dump(sbi);
		break;
	case DEFRAG:
		if (do_defrag(sbi))
			goto out_err;
		break;
	}

	f2fs_do_umount(sbi);

	if (config.func == FSCK && config.bug_on) {
		if (!config.ro && config.fix_on == 0 && config.auto_fix == 0) {
			char ans[255] = {0};
retry:
			printf("Do you want to fix this partition? [Y/N] ");
			ret = scanf("%s", ans);
			ASSERT(ret >= 0);
			if (!strcasecmp(ans, "y"))
				config.fix_on = 1;
			else if (!strcasecmp(ans, "n"))
				config.fix_on = 0;
			else
				goto retry;

			if (config.fix_on)
				goto fsck_again;
		}
	}
	f2fs_finalize_device(&config);
out:
	/* get end time */
	gettimeofday(&t_end, NULL);
	end_ms = ((long)t_end.tv_sec)*1000 + (long)t_end.tv_usec/1000;
	printf("\nEnd time: %ld ms\n", end_ms);

	/* calculate time */
	cost_ms = end_ms - start_ms;
	printf("Cost time: %ld ms\n", cost_ms);

	printf("\nDone.\n");
	return 0;

out_err:
	if (sbi->ckpt)
		free(sbi->ckpt);
	if (sbi->raw_super)
		free(sbi->raw_super);
	return -1;
}
