; ModuleID = 'mini-c'
source_filename = "mini-c"

@a = common global i32 0, align 4
@b = common global float 0.000000e+00, align 4
@c = common global i1 false, align 1
@o = common global i32 0, align 4
@globool = common global i1 false, align 1

define float @asd() {
entry:
  %a = alloca float, align 4
  %load_temp = load float, ptr %a, align 4
  ret float %load_temp
}

define i1 @test(i32 %a, i32 %b, i1 %c) {
entry:
  %aaa = alloca i1, align 1
  %test = alloca float, align 4
  %c3 = alloca i1, align 1
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  store i1 %c, ptr %c3, align 1
  store float 5.000000e+00, ptr %test, align 4
  store i1 false, ptr %aaa, align 1
  %load_temp = load i32, ptr %a1, align 4
  %load_temp4 = load i32, ptr %b2, align 4
  %add_tmp = add i32 %load_temp, %load_temp4
  %load_temp5 = load i1, ptr %c3, align 1
  %0 = zext i1 %load_temp5 to i32
  %add_tmp6 = add i32 %add_tmp, %0
  %itof_cast = sitofp i32 %add_tmp6 to float
  store float %itof_cast, ptr %test, align 4
  %load_temp7 = load i1, ptr %aaa, align 1
  ret i1 %load_temp7
}

define i32 @addition(i32 %n, i32 %m) {
entry:
  %result = alloca i32, align 4
  %m2 = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store i32 %m, ptr %m2, align 4
  %load_temp = load i32, ptr %n1, align 4
  %load_temp3 = load i32, ptr %m2, align 4
  %add_tmp = add i32 %load_temp, %load_temp3
  store i32 %add_tmp, ptr %result, align 4
  %load_temp4 = load i32, ptr %result, align 4
  ret i32 %load_temp4
}

define void @tester(i32 %v) {
entry:
  %v1 = alloca i32, align 4
  store i32 %v, ptr %v1, align 4
  ret void
}

define i32 @ret(float %h) {
entry:
  %h1 = alloca float, align 4
  store float %h, ptr %h1, align 4
  ret i32 0
}

define i32 @main() {
entry:
  %hg = alloca i32, align 4
  %hg_o = alloca i32, align 4
  %A = alloca i32, align 4
  %flo = alloca i32, align 4
  %ty = alloca i32, align 4
  %o = alloca i32, align 4
  %combo = alloca float, align 4
  %load_temp = load i32, ptr %o, align 4
  store i32 %load_temp, ptr %ty, align 4
  %load_global_temp = load i32, ptr @a, align 4
  %eq_tmp = icmp eq i32 %load_global_temp, 10
  %if_cond = icmp ne i1 %eq_tmp, false
  br i1 %if_cond, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  store i32 99, ptr %hg_o, align 4
  br label %while_cond

if_else:                                          ; preds = %entry
  store i32 190, ptr %flo, align 4
  br label %if_end14

while_cond:                                       ; preds = %if_end, %if_then
  %load_temp1 = load i32, ptr %ty, align 4
  %eq_tmp2 = icmp eq i32 %load_temp1, 10
  %if_cond3 = icmp ne i1 %eq_tmp2, false
  br i1 %if_cond3, label %while_body, label %while_end13

while_body:                                       ; preds = %while_cond
  store i32 0, ptr %o, align 4
  %load_temp6 = load i32, ptr %ty, align 4
  %ne_tmp = icmp ne i32 %load_temp6, 19
  %if_cond7 = icmp ne i1 %ne_tmp, false
  br i1 %if_cond7, label %if_then4, label %if_else5

if_then4:                                         ; preds = %while_body
  br label %while_cond8

if_else5:                                         ; preds = %while_body
  store i32 10, ptr %ty, align 4
  store i32 0, ptr %hg_o, align 4
  br label %if_end

while_cond8:                                      ; preds = %if_then4
  %load_global_temp10 = load i32, ptr @a, align 4
  %eq_tmp11 = icmp eq i32 %load_global_temp10, 10
  %if_cond12 = icmp ne i1 %eq_tmp11, false
  br i1 %if_cond12, label %while_body9, label %while_end

while_body9:                                      ; preds = %while_cond8
  ret i32 0

while_end:                                        ; preds = %while_cond8
  store i32 0, ptr %hg, align 4
  br label %if_end

if_end:                                           ; preds = %if_else5, %while_end
  br label %while_cond

while_end13:                                      ; preds = %while_cond
  store i32 10, ptr %o, align 4
  br label %if_end14

if_end14:                                         ; preds = %if_else, %while_end13
  call void @tester(i32 0)
  %load_temp15 = load i32, ptr %o, align 4
  %neg_temp = sub i32 0, %load_temp15
  store i32 %neg_temp, ptr %o, align 4
  %load_global_temp16 = load i1, ptr @globool, align 1
  %not_temp = xor i1 %load_global_temp16, true
  %btoi_cast = zext i1 %not_temp to i32
  store i32 %btoi_cast, ptr %o, align 4
  store float 1.100000e+01, ptr %combo, align 4
  store i32 5, ptr @a, align 4
  %load_global_temp17 = load i32, ptr @a, align 4
  %itof_cast = sitofp i32 %load_global_temp17 to float
  store float %itof_cast, ptr @b, align 4
  %load_temp18 = load i32, ptr %o, align 4
  %sub_tmp = sub i32 %load_temp18, -9
  %itof_cast19 = sitofp i32 %sub_tmp to float
  store float %itof_cast19, ptr %combo, align 4
  store float 5.000000e+00, ptr %combo, align 4
  ret i32 0
}
