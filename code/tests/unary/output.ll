; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

declare float @print_float(float)

define float @unary(i32 %n, float %m) {
entry:
  %sum = alloca float, align 4
  %result = alloca float, align 4
  %m2 = alloca float, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store float %m, ptr %m2, align 4
  store float 0.000000e+00, ptr %sum, align 4
  %load_temp = load i32, ptr %n1, align 4
  %load_temp3 = load float, ptr %m2, align 4
  %0 = sitofp i32 %load_temp to float
  %fadd_tmp = fadd float %0, %load_temp3
  store float %fadd_tmp, ptr %result, align 4
  %load_arg = load float, ptr %result, align 4
  %call_tmp = call float @print_float(float %load_arg)
  %load_temp4 = load float, ptr %sum, align 4
  %load_temp5 = load float, ptr %result, align 4
  %fadd_tmp6 = fadd float %load_temp4, %load_temp5
  store float %fadd_tmp6, ptr %sum, align 4
  %load_temp7 = load float, ptr %m2, align 4
  %fneg_temp = fneg float %load_temp7
  %load_temp8 = load i32, ptr %n1, align 4
  %1 = sitofp i32 %load_temp8 to float
  %fadd_tmp9 = fadd float %1, %fneg_temp
  store float %fadd_tmp9, ptr %result, align 4
  %load_arg10 = load float, ptr %result, align 4
  %call_tmp11 = call float @print_float(float %load_arg10)
  %load_temp12 = load float, ptr %sum, align 4
  %load_temp13 = load float, ptr %result, align 4
  %fadd_tmp14 = fadd float %load_temp12, %load_temp13
  store float %fadd_tmp14, ptr %sum, align 4
  %load_temp15 = load float, ptr %m2, align 4
  %fneg_temp16 = fneg float %load_temp15
  %fneg_temp17 = fneg float %fneg_temp16
  %load_temp18 = load i32, ptr %n1, align 4
  %2 = sitofp i32 %load_temp18 to float
  %fadd_tmp19 = fadd float %2, %fneg_temp17
  store float %fadd_tmp19, ptr %result, align 4
  %load_arg20 = load float, ptr %result, align 4
  %call_tmp21 = call float @print_float(float %load_arg20)
  %load_temp22 = load float, ptr %sum, align 4
  %load_temp23 = load float, ptr %result, align 4
  %fadd_tmp24 = fadd float %load_temp22, %load_temp23
  store float %fadd_tmp24, ptr %sum, align 4
  %load_temp25 = load i32, ptr %n1, align 4
  %neg_temp = sub i32 0, %load_temp25
  %load_temp26 = load float, ptr %m2, align 4
  %fneg_temp27 = fneg float %load_temp26
  %3 = sitofp i32 %neg_temp to float
  %fadd_tmp28 = fadd float %3, %fneg_temp27
  store float %fadd_tmp28, ptr %result, align 4
  %load_arg29 = load float, ptr %result, align 4
  %call_tmp30 = call float @print_float(float %load_arg29)
  %load_temp31 = load float, ptr %sum, align 4
  %load_temp32 = load float, ptr %result, align 4
  %fadd_tmp33 = fadd float %load_temp31, %load_temp32
  store float %fadd_tmp33, ptr %sum, align 4
  %load_temp34 = load float, ptr %sum, align 4
  ret float %load_temp34
}
