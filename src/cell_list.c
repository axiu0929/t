/*
Command: AT+QOPS

Response:

For 2G:
+QOPS: <oper_in_string>,<oper_in_short_string>,<oper_in_number>
<index>,<RAT,<freq>,<lac>,<ci>,<bsic>,<rxlev>,<c1>,<cba>,<is_gprs_support>
[...]

OK
we need: <oper_in_number>,lac,ci,rxlev

For 3G:
+QOPS: <oper_in_string>,<oper_in_short_string>,<oper_in_number>
<index>,<RAT>,<freq>,<psc>,<lac>,<ci>,<rscp>,<ecno>,<cba>
[...]

OK
we need: <oper_in_number>,lac,ci,rscp

For 4G:
+QOPS: <oper_in_string>,<oper_in_short_string>,<oper_in_number>
<index>,<RAT>,<freq>,<pci>,<tac>,<ci>,<rsrp>,<rsrq>,<cba>
[...]

OK
we need: <oper_in_number>,tac,ci,rsrp
*/

// response 存到 response.h 
// 用 fscanf 逐行讀取檔案 from response.h

/*
loop until OK

if +: 讀到第2個, 存<oper_in_number>直到 0x0A or 0x0D

else:
2G:
讀到第3個, 存<lac>直到, 存<ci>直到, 再讀到1個, 存<rxlev>直到, 讀到0x0A or 0x0D
3G:
讀到第4個, 存<lac>直到, 存<ci>直到, 存<rscp>直到, 讀到0x0A or 0x0D
4G:
讀到第4個, 存<tac>直到, 存<ci>直到, 存<rsrp>直到, 讀到0x0A or 0x0D
*/

/*
void make_cell_list(int RAT)
{
    scancontrol(RAT);

    
}
*/