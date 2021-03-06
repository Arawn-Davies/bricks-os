; Generated by VUC 2004.1

.init_vf_all
.init_vi_all
.syntax new

.vu

--enter
--endenter

.include "vcl_sml.i"
.include "std.i"

.macro _TransformVertex vertexAddr_004 saveAddr_005 matrix_006 
  lq             vertex_001, 0(\vertexAddr_004)
  iaddiu         \vertexAddr_004, \vertexAddr_004, 1
  _MatrixMultiplyVertex vertex_001 \matrix_006 vertex_001 
  _ftoi4         vertex_001 vertex_001 
  sq             vertex_001, 0(\saveAddr_005)
  iaddiu         \saveAddr_005, \saveAddr_005, 1
L_001_RETURN\@:
.endm

.macro _main vuMem_007 
  _xtop          dataAddr_008 
  _GenericArrayLoad matrix_009, dataAddr_008, 0, 4
  iaddiu         vertexAddr_010, dataAddr_008, 5
  iaddiu         primAddr_011, dataAddr_008, 256
  iaddiu         saveAddr_012, dataAddr_008, 257
  iaddiu         vertCount_002, vi00, 3
  lq             tmp_035, 4(dataAddr_008)
  sq             tmp_035, 0(primAddr_011)
  iaddiu         i_003, vi00, 0
L_004_FOR_START\@:
  isub           cmp_040, i_003, vertCount_002
  ibltz          cmp_040, L_005_CMP_TRUE\@
  iaddiu         tmp_039, vi00, 0
  b              L_006_CMP_END\@
L_005_CMP_TRUE\@:
  iaddiu         tmp_039, vi00, 1
L_006_CMP_END\@:
  ibeq           tmp_039, vi00, L_003_FOR_END\@
  _TransformVertex vertexAddr_010 saveAddr_012 matrix_009 
  iaddiu         i_003, i_003, 1
  b              L_004_FOR_START\@
L_003_FOR_END\@:
  _xgkick        primAddr_011 
L_002_RETURN\@:
.endm

VUC_BEGIN:
  _main vi00
VUC_END:

--exit
--endexit
