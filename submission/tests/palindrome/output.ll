; ModuleID = 'mini-c'
source_filename = "mini-c"

define i1 @palindrome(i32 %number) {
entry:
  %result = alloca i1, align 1
  %rmndr = alloca i32, align 4
  %rev = alloca i32, align 4
  %t = alloca i32, align 4
  %number1 = alloca i32, align 4
  store i32 %number, ptr %number1, align 4
  store i32 0, ptr %rev, align 4
  store i1 false, ptr %result, align 1
  %load_temp = load i32, ptr %number1, align 4
  store i32 %load_temp, ptr %t, align 4
  br label %while_cond

while_cond:                                       ; preds = %while_body, %entry
  %load_temp2 = load i32, ptr %number1, align 4
  %gt_tmp = icmp sgt i32 %load_temp2, 0
  %if_cond = icmp ne i1 %gt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp3 = load i32, ptr %number1, align 4
  %mod_tmp = srem i32 %load_temp3, 10
  store i32 %mod_tmp, ptr %rmndr, align 4
  %load_temp4 = load i32, ptr %rev, align 4
  %mul_tmp = mul i32 %load_temp4, 10
  %load_temp5 = load i32, ptr %rmndr, align 4
  %add_tmp = add i32 %mul_tmp, %load_temp5
  store i32 %add_tmp, ptr %rev, align 4
  %load_temp6 = load i32, ptr %number1, align 4
  %div_tmp = sdiv i32 %load_temp6, 10
  store i32 %div_tmp, ptr %number1, align 4
  br label %while_cond

while_end:                                        ; preds = %while_cond
  %load_temp7 = load i32, ptr %t, align 4
  %load_temp8 = load i32, ptr %rev, align 4
  %eq_tmp = icmp eq i32 %load_temp7, %load_temp8
  %if_cond9 = icmp ne i1 %eq_tmp, false
  br i1 %if_cond9, label %if_then, label %if_else

if_then:                                          ; preds = %while_end
  store i1 true, ptr %result, align 1
  br label %if_end

if_else:                                          ; preds = %while_end
  store i1 false, ptr %result, align 1
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %load_temp10 = load i1, ptr %result, align 1
  ret i1 %load_temp10
}
