dtb-y += NEXT_L09_V4_config.dtb
dtb-y += NEXT_L29_VN1_2_config.dtb
dtb-y += NEXT_CLG_VN2_2_config.dtb
dtb-y += NEXT_L29_VN1_3_config.dtb
dtb-y += NEXT_L29_VN1_config.dtb
dtb-y += NEXT_TUGL_V3_config.dtb
dtb-y += NEXT_CLG_VN1_config.dtb
dtb-y += NEXT_TUGL_VN1_config.dtb
dtb-y += NEXT_TUGL_VN1_2_config.dtb
dtb-y += NEXT_CLG_V3_config.dtb
dtb-y += NEXT_TUGL_V4_3_config.dtb
dtb-y += NEXT_TUGL_V4_2_config.dtb
dtb-y += NEXT_TUCL_VN2_config.dtb
dtb-y += NEXT_TUCL_V3_config.dtb
dtb-y += NEXT_TUCL_V1_config.dtb
dtb-y += NEXT_TUCL_V4_2_config.dtb
dtb-y += NEXT_L29_VN2_config.dtb
dtb-y += NEXT_TUGL_V4_config.dtb
dtb-y += NEXT_L09_VN1_2_config.dtb
dtb-y += NEXT_TUGL_VN2_config.dtb
dtb-y += NEXT_CLG_VN2_config.dtb
dtb-y += NEXT_TUCL_VN1_config.dtb
dtb-y += NEXT_TUCL_VN1_2_config.dtb
dtb-y += NEXT_TUCL_V4_config.dtb
dtb-y += NEXT_L09_VN2_config.dtb
dtb-y += NEXT_TUCL_V0_config.dtb
dtb-y += NEXT_TUCL_V4_3_config.dtb
dtb-y += NEXT_TUCL_V3E_config.dtb
dtb-y += NEXT_L09_VN1_config.dtb
dtb-y += NEXT_L09_V1_config.dtb
dtb-y += NEXT_L29_V4_config.dtb
dtb-y += NEXT_CLG_V4_config.dtb

targets += dtbs

dtbs: $(addprefix $(obj)/, $(dtb-y))
	$(Q)rm -f $(obj)/../*.dtb

clean-files := *.dtb
