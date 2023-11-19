; ModuleID = 'mini-c'
source_filename = "mini-c"

declare float @print_float(float)

define float @cosine(float %x) {
entry:
  %alt = alloca float, align 4
  %eps = alloca float, align 4
  %term = alloca float, align 4
  %n = alloca float, align 4
  %cos = alloca float, align 4
  %x1 = alloca float, align 4
  store float %x, ptr %x1, align 4
  store float 0x3EB0C6F7A0000000, ptr %eps, align 4
  store float 1.000000e+00, ptr %n, align 4
  store float 1.000000e+00, ptr %cos, align 4
  store float 1.000000e+00, ptr %term, align 4
  store float -1.000000e+00, ptr %alt, align 4
  br label %while_cond

while_cond:                                       ; preds = %while_body, %entry
  %load_temp = load float, ptr %term, align 4
  %load_temp2 = load float, ptr %eps, align 4
  %fgt_tmp = fcmp ogt float %load_temp, %load_temp2
  %if_cond = icmp ne i1 %fgt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp3 = load float, ptr %term, align 4
  %load_temp4 = load float, ptr %x1, align 4
  %fmul_tmp = fmul float %load_temp3, %load_temp4
  %load_temp5 = load float, ptr %x1, align 4
  %fmul_tmp6 = fmul float %fmul_tmp, %load_temp5
  %load_temp7 = load float, ptr %n, align 4
  %fdiv_tmp = fdiv float %fmul_tmp6, %load_temp7
  %load_temp8 = load float, ptr %n, align 4
  %fadd_tmp = fadd float %load_temp8, 1.000000e+00
  %fdiv_tmp9 = fdiv float %fdiv_tmp, %fadd_tmp
  store float %fdiv_tmp9, ptr %term, align 4
  %load_temp10 = load float, ptr %alt, align 4
  %load_temp11 = load float, ptr %term, align 4
  %fmul_tmp12 = fmul float %load_temp10, %load_temp11
  %load_temp13 = load float, ptr %cos, align 4
  %fadd_tmp14 = fadd float %load_temp13, %fmul_tmp12
  store float %fadd_tmp14, ptr %cos, align 4
  %load_temp15 = load float, ptr %alt, align 4
  %fneg_temp = fneg float %load_temp15
  store float %fneg_temp, ptr %alt, align 4
  %load_temp16 = load float, ptr %n, align 4
  %fadd_tmp17 = fadd float %load_temp16, 2.000000e+00
  store float %fadd_tmp17, ptr %n, align 4
  br label %while_cond

while_end:                                        ; preds = %while_cond
  %load_arg = load float, ptr %cos, align 4
  %call_tmp = call float @print_float(float %load_arg)
  %load_temp18 = load float, ptr %cos, align 4
  ret float %load_temp18
}
