; ModuleID = 'mini-c'
source_filename = "mini-c"

define float @pi() {
entry:
  %i = alloca i32, align 4
  %PI = alloca float, align 4
  %flag = alloca i1, align 1
  store i1 true, ptr %flag, align 1
  store float 3.000000e+00, ptr %PI, align 4
  store i32 2, ptr %i, align 4
  br label %while_cond

while_cond:                                       ; preds = %if_end, %entry
  %load_temp = load i32, ptr %i, align 4
  %lt_tmp = icmp slt i32 %load_temp, 100
  %if_cond = icmp ne i1 %lt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp1 = load i1, ptr %flag, align 1
  %if_cond2 = icmp ne i1 %load_temp1, false
  br i1 %if_cond2, label %if_then, label %if_else

if_then:                                          ; preds = %while_body
  %load_temp3 = load i32, ptr %i, align 4
  %add_tmp = add i32 %load_temp3, 1
  %load_temp4 = load i32, ptr %i, align 4
  %mul_tmp = mul i32 %load_temp4, %add_tmp
  %load_temp5 = load i32, ptr %i, align 4
  %add_tmp6 = add i32 %load_temp5, 2
  %mul_tmp7 = mul i32 %mul_tmp, %add_tmp6
  %0 = sitofp i32 %mul_tmp7 to float
  %fdiv_tmp = fdiv float 4.000000e+00, %0
  %load_temp8 = load float, ptr %PI, align 4
  %fadd_tmp = fadd float %load_temp8, %fdiv_tmp
  store float %fadd_tmp, ptr %PI, align 4
  br label %if_end

if_else:                                          ; preds = %while_body
  %load_temp9 = load i32, ptr %i, align 4
  %add_tmp10 = add i32 %load_temp9, 1
  %load_temp11 = load i32, ptr %i, align 4
  %mul_tmp12 = mul i32 %load_temp11, %add_tmp10
  %load_temp13 = load i32, ptr %i, align 4
  %add_tmp14 = add i32 %load_temp13, 2
  %mul_tmp15 = mul i32 %mul_tmp12, %add_tmp14
  %1 = sitofp i32 %mul_tmp15 to float
  %fdiv_tmp16 = fdiv float 4.000000e+00, %1
  %load_temp17 = load float, ptr %PI, align 4
  %fsub_tmp = fsub float %load_temp17, %fdiv_tmp16
  store float %fsub_tmp, ptr %PI, align 4
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %load_temp18 = load i1, ptr %flag, align 1
  %not_temp = xor i1 %load_temp18, true
  store i1 %not_temp, ptr %flag, align 1
  %load_temp19 = load i32, ptr %i, align 4
  %add_tmp20 = add i32 %load_temp19, 2
  store i32 %add_tmp20, ptr %i, align 4
  br label %while_cond

while_end:                                        ; preds = %while_cond
  %load_temp21 = load float, ptr %PI, align 4
  ret float %load_temp21
}
