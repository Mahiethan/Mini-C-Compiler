; ModuleID = 'mini-c'
source_filename = "mini-c"

define i32 @multiplyNumbers(i32 %n) {
entry:
  %result = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store i32 0, ptr %result, align 4
  %load_temp = load i32, ptr %n1, align 4
  %ge_tmp = icmp sge i32 %load_temp, 1
  %if_cond = icmp ne i1 %ge_tmp, false
  br i1 %if_cond, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  %load_temp2 = load i32, ptr %n1, align 4
  %sub_tmp = sub i32 %load_temp2, 1
  %call_tmp = call i32 @multiplyNumbers(i32 %sub_tmp)
  %load_temp3 = load i32, ptr %n1, align 4
  %mul_tmp = mul i32 %load_temp3, %call_tmp
  store i32 %mul_tmp, ptr %result, align 4
  br label %if_end

if_else:                                          ; preds = %entry
  store i32 1, ptr %result, align 4
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %load_temp4 = load i32, ptr %result, align 4
  ret i32 %load_temp4
}

define i32 @rfact(i32 %n) {
entry:
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  %load_arg = load i32, ptr %n1, align 4
  %call_tmp = call i32 @multiplyNumbers(i32 %load_arg)
  ret i32 %call_tmp
}
