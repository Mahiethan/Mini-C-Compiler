; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

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
  %load_temp4 = load i32, ptr %n1, align 4
  %eq_tmp = icmp eq i32 %load_temp4, 4
  %if_cond = icmp ne i1 %eq_tmp, false
  br i1 %if_cond, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  %load_temp5 = load i32, ptr %n1, align 4
  %load_temp6 = load i32, ptr %m2, align 4
  %add_tmp7 = add i32 %load_temp5, %load_temp6
  %call_tmp = call i32 @print_int(i32 %add_tmp7)
  br label %if_end

if_else:                                          ; preds = %entry
  %load_temp8 = load i32, ptr %n1, align 4
  %load_temp9 = load i32, ptr %m2, align 4
  %mul_tmp = mul i32 %load_temp8, %load_temp9
  %call_tmp10 = call i32 @print_int(i32 %mul_tmp)
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %load_temp11 = load i32, ptr %result, align 4
  ret i32 %load_temp11
}
