// RUN: mlir-opt -split-input-file \
// RUN: -transform-preload-library='transform-library-paths=%p/../td/vectorize-with-patterns.mlir' \
// RUN: -transform-interpreter=entry-point=vectorize_with_patterns %s | FileCheck %s

//===----------------------------------------------------------------------===//
// Contiguous load
//===----------------------------------------------------------------------===//

#map = affine_map<(d0, d1, d2) -> (d0, d1, d2)>
func.func @vectorize_nd_tensor_extract_transfer_read_basic(
    %arg0: tensor<3x3x3xf32>,
    %arg1: tensor<1x1x3xf32>) -> tensor<1x1x3xf32> {

  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel", "parallel", "parallel"]
  } outs(%arg1 : tensor<1x1x3xf32>) {
  ^bb0(%out: f32):
    %1 = linalg.index 0 : index
    %2 = linalg.index 1 : index
    %3 = linalg.index 2 : index
    %4 = tensor.extract %arg0[%1, %2, %3] : tensor<3x3x3xf32>
    linalg.yield %4 : f32
  } -> tensor<1x1x3xf32>

  return %res : tensor<1x1x3xf32>
}

// CHECK-LABEL: func.func @vectorize_nd_tensor_extract_transfer_read_basic
// CHECK-SAME: %[[ARG0:.*]]: tensor<3x3x3xf32>
// CHECK-SAME: %[[ARG1:.*]]: tensor<1x1x3xf32>

// CHECK-DAG:  %[[C0:.+]] = arith.constant 0 : index
// CHECK-DAG:  %[[PV:.+]] = ub.poison : f32

// CHECK:   %[[READ:.*]] = vector.transfer_read %[[ARG0]][%[[C0]], %[[C0]], %[[C0]]], %[[PV]] {in_bounds = [true, true, true]} : tensor<3x3x3xf32>, vector<1x1x3xf32>
// CHECK:   vector.transfer_write %[[READ]], %[[ARG1]][%[[C0]], %[[C0]], %[[C0]]] {in_bounds = [true, true, true]} : vector<1x1x3xf32>, tensor<1x1x3xf32>

 // -----

func.func @vectorize_nd_tensor_extract_transfer_read_complex(%6: tensor<45x80x16xf32>, %arg0: index, %arg2: index, %arg1: index, %arg4: index, %extracted_slice : tensor<1x4xf32>) -> tensor<1x4xf32> {
  %c79 = arith.constant 79 : index
  %25 = linalg.generic {
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>],
    iterator_types = ["parallel", "parallel"]
  } outs(%extracted_slice : tensor<1x4xf32>) {
  ^bb0(%out: f32):
    %26 = linalg.index 0 : index
    %27 = arith.addi %arg0, %26 : index
    %28 = arith.addi %27, %arg2 : index
    %29 = linalg.index 1 : index
    %30 = arith.addi %arg1, %29 : index
    %31 = arith.addi %30, %arg4 : index
    %extracted = tensor.extract %6[%28, %c79, %31] : tensor<45x80x16xf32>
    linalg.yield %extracted : f32
  } -> tensor<1x4xf32>
  return %25 : tensor<1x4xf32>
}


// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_transfer_read_complex(
// CHECK-SAME:      %[[ARG0:.*]]: tensor<45x80x16xf32>,
// CHECK-SAME:      %[[ARG1:.*]]: index, %[[ARG2:.*]]: index, %[[ARG3:.*]]: index, %[[ARG4:.*]]: index,
// CHECK-SAME:      %[[ARG5:.*]]: tensor<1x4xf32>) -> tensor<1x4xf32> {

// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[PV:.*]] = ub.poison : f32
// CHECK-DAG:       %[[C79:.*]] = arith.constant 79 : index
// CHECK:           %[[ADD1:.*]] = arith.addi %[[ARG1]], %[[ARG2]] : index
// CHECK:           %[[ADD2:.*]] = arith.addi %[[ARG3]], %[[ARG4]] : index

// CHECK:           %[[READ:.*]] = vector.transfer_read %[[ARG0]]{{\[}}%[[ADD1]], %[[C79]], %[[ADD2]]], %[[PV]] {in_bounds = [true, true]} : tensor<45x80x16xf32>, vector<1x4xf32>
// CHECK:           %[[WRITE:.*]] = vector.transfer_write %[[READ]], %[[ARG5]]{{\[}}%[[C0]], %[[C0]]] {in_bounds = [true, true]} : vector<1x4xf32>, tensor<1x4xf32>
// CHECK:           return %[[WRITE]] : tensor<1x4xf32>
// CHECK:         }

// -----

// The vectorizer converts `affine.apply` so that the subsequent Ops can be vectorised based on the converted ops. Contiguous load.
func.func @vectorize_nd_tensor_extract_with_affine_apply_contiguous(%6: tensor<80x16xf32>, %arg0: index, %extracted_slice : tensor<1x4xf32>) -> tensor<1x4xf32> {
  %c79 = arith.constant 79 : index
  %1 = linalg.generic {
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>],
    iterator_types = ["parallel", "parallel"]
  } outs(%extracted_slice : tensor<1x4xf32>) {
  ^bb0(%out: f32):
    %2 = linalg.index 1 : index
    %3 = affine.apply affine_map<(d0, d1) -> (d0 + d1)>(%2, %arg0)
    %extracted = tensor.extract %6[%c79, %3] : tensor<80x16xf32>
    linalg.yield %extracted : f32
  } -> tensor<1x4xf32>
  return %1 : tensor<1x4xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_with_affine_apply_contiguous(
// CHECK-SAME:      %[[ARG0:.*]]: tensor<80x16xf32>,
// CHECK-SAME:      %[[ARG1:.*]]: index,
// CHECK-SAME:      %[[ARG2:.*]]: tensor<1x4xf32>) -> tensor<1x4xf32> {

// CHECK-DAG:       %[[PV:.*]] = ub.poison : f32
// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[C79:.*]] = arith.constant 79 : index

// CHECK:           %[[READ:.*]] = vector.transfer_read %[[ARG0]]{{\[}}%[[C79]], %[[ARG1]]], %[[PV]] {in_bounds = [true, true]} : tensor<80x16xf32>, vector<1x4xf32>
// CHECK:           %[[WRITE:.*]] = vector.transfer_write %[[READ]], %[[ARG2]]{{\[}}%[[C0]], %[[C0]]] {in_bounds = [true, true]} : vector<1x4xf32>, tensor<1x4xf32>
// CHECK:           return %[[WRITE]] : tensor<1x4xf32>
// CHECK:         }

// -----

func.func @vectorize_nd_tensor_extract_with_tensor_extract(%input_1: tensor<1x20xi32>, %input_2: tensor<257x24xf32>, %arg0 : index, %arg1 : index, %arg2 : index, %arg3 : index) -> tensor<1x1x4xf32> {
  %c0 = arith.constant 0 : index
  %c256 = arith.constant 256 : index
  %output = tensor.empty() : tensor<1x1x4xf32>
  %1 = linalg.generic {indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>], iterator_types = ["parallel", "parallel", "parallel"]} outs(%output : tensor<1x1x4xf32>) {
  ^bb0(%out: f32):
    %13 = linalg.index 0 : index
    %14 = affine.apply affine_map<(d0, d1, d2) -> (d0 + d1 + d2)>(%arg0, %13, %arg2)
    %15 = linalg.index 2 : index
    %16 = linalg.index 1 : index
    %17 = affine.apply affine_map<(d0, d1, d2, d3) -> (d0 + d1 * 24 + d2 + d3)>(%arg1, %16, %15, %arg3)
    %extracted_0 = tensor.extract %input_1[%c0, %14] : tensor<1x20xi32>
    %18 = arith.index_cast %extracted_0 : i32 to index
    %19 = arith.maxsi %18, %c0 : index
    %20 = arith.minsi %19, %c256 : index
    %extracted_1 = tensor.extract %input_2[%20, %17] : tensor<257x24xf32>
    linalg.yield %extracted_1 : f32
  } -> tensor<1x1x4xf32>
  return %1 : tensor<1x1x4xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_with_tensor_extract(
// CHECK-SAME:      %[[INPUT_1:.*]]: tensor<1x20xi32>,
// CHECK-SAME:      %[[INPUT_2:.*]]: tensor<257x24xf32>,
// CHECK-SAME:      %[[INPUT_3:.*]]: index, %[[INPUT_4:.*]]: index, %[[INPUT_5:.*]]: index,
// CHECK:           %[[EXTRACTED_0_IDX_0:.*]] = arith.constant 0 : index
// CHECK:           %[[SCALAR:.*]] = arith.addi %[[INPUT_3]], %[[INPUT_5]] : index
// First `vector.transfer_read` from the generic Op - loop invariant scalar load.
// CHECK:           vector.transfer_read %[[INPUT_1]][%[[EXTRACTED_0_IDX_0]], %[[SCALAR]]]
// CHECK-SAME:      tensor<1x20xi32>, vector<i32>
// The following `tensor.extract` from the generic Op s a contiguous load (all Ops used
// for address calculation also satisfy the required conditions).
// CHECK:           vector.transfer_read %[[INPUT_2]][%{{.*}}, %{{.*}}, %{{.*}} {in_bounds = [true, true]} : tensor<257x24xf32>, vector<1x4xf32>

// Make sure that non-linear arithmetic operations (e.g. arith.maxsi) are allowed when calculating indices for load operations. Contiguous load.
func.func @vectorize_nd_tensor_extract_with_maxsi_contiguous(%arg0: tensor<80x16xf32>, %extracted_slice : tensor<1x4xf32>) -> tensor<1x4xf32> {
  %c16 = arith.constant 16 : index
  %1 = linalg.generic {
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>],
    iterator_types = ["parallel", "parallel"]
  } outs(%extracted_slice : tensor<1x4xf32>) {
  ^bb0(%out: f32):
    %2 = linalg.index 0 : index
    %3 = linalg.index 1 : index
    %4 = arith.maxsi %2, %c16 : index
    %extracted = tensor.extract %arg0[%4, %3] : tensor<80x16xf32>
    linalg.yield %extracted : f32
  } -> tensor<1x4xf32>
  return %1 : tensor<1x4xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_with_maxsi_contiguous(
// CHECK-SAME:                                                                 %[[VAL_0:.*]]: tensor<80x16xf32>,
// CHECK-SAME:                                                                 %[[VAL_1:.*]]: tensor<1x4xf32>) -> tensor<1x4xf32> {
// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[C16:.*]] = arith.constant 16 : index
// CHECK-DAG:       %[[PV:.*]] = ub.poison : f32

// CHECK:           %[[VAL_8:.*]] = vector.transfer_read %[[VAL_0]]{{\[}}%[[C16]], %[[C0]]], %[[PV]] {in_bounds = [true, true]} : tensor<80x16xf32>, vector<1x4xf32>
// CHECK:           %[[VAL_9:.*]] = vector.transfer_write %[[VAL_8]], %[[VAL_1]]{{\[}}%[[C0]], %[[C0]]] {in_bounds = [true, true]} : vector<1x4xf32>, tensor<1x4xf32>
// CHECK:           return %[[VAL_9]] : tensor<1x4xf32>
// CHECK:         }

// -----

//===----------------------------------------------------------------------===//
// Gather load
//===----------------------------------------------------------------------===//

#map0 = affine_map<(d0, d1, d2, d3) -> (d0, d2)>
#map1 = affine_map<(d0, d1, d2, d3) -> (d0, d1, d2, d3)>
func.func @vectorize_1d_tensor_extract(%arg0: tensor<3xf32>, %arg1: tensor<4x3xi32>, %arg2: tensor<4x7x3x2xf32>) -> tensor<4x7x3x2xf32> {
  %1 = linalg.generic {
    indexing_maps = [#map0, #map1],
    iterator_types = ["parallel", "parallel", "parallel", "parallel"]
  } ins(%arg1 : tensor<4x3xi32>) outs(%arg2 : tensor<4x7x3x2xf32>) {
  ^bb0(%arg3: i32, %arg4: f32):
    %2 = arith.index_cast %arg3 : i32 to index
    %3 = tensor.extract %arg0[%2] : tensor<3xf32>
    linalg.yield %3 : f32
  } -> tensor<4x7x3x2xf32>
  return %1 : tensor<4x7x3x2xf32>
}
// CHECK-LABEL: func.func @vectorize_1d_tensor_extract
// CHECK-SAME:    %[[ARG0:.*]]: tensor<3xf32>
// CHECK-SAME:    %[[ARG1:.*]]: tensor<4x3xi32>
// CHECK-DAG: %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG: %[[MASK:.*]] = arith.constant dense<true> : vector<4x7x3x2xi1>
// CHECK-DAG: %[[PASSTHRU:.*]] = arith.constant dense<0.000000e+00> : vector<4x7x3x2xf32>
// CHECK: %[[V0:.*]] = vector.transfer_read %[[ARG1]]
// CHECK: %[[CAST:.*]] = arith.index_cast %[[V0]]
// CHECK: %[[BROADCAST:.*]] = vector.broadcast %[[CAST]]
// CHECK: %[[INDICES:.*]] = vector.transpose %[[BROADCAST]]
// CHECK: %[[GATHER:.*]] = vector.gather %[[ARG0]][%[[C0]]] [%[[INDICES]]], %[[MASK]], %[[PASSTHRU]]
// CHECK: vector.transfer_write %[[GATHER]]

// -----

#map0 = affine_map<(d0, d1, d2, d3) -> (d0, d2)>
#map1 = affine_map<(d0, d1, d2, d3) -> (d0, d1, d3)>
#map2 = affine_map<(d0, d1, d2, d3) -> (d0, d1, d2, d3)>
func.func @vectorize_nd_tensor_extract_index_from_tensor(%arg0: tensor<3x3xf32>, %arg1: tensor<4x3xi32>, %arg2: tensor<4x3xi32>, %arg3: tensor<4x7x2xf32>, %arg4: tensor<4x7x3x2xf32>) -> tensor<4x7x3x2xf32> {
  %2 = linalg.generic {
    indexing_maps = [#map0, #map0, #map1, #map2],
    iterator_types = ["parallel", "parallel", "parallel", "parallel"]
  } ins(%arg1, %arg2, %arg3 : tensor<4x3xi32>, tensor<4x3xi32>, tensor<4x7x2xf32>) outs(%arg4 : tensor<4x7x3x2xf32>) {
  ^bb0(%arg5: i32, %arg6: i32, %arg7: f32, %arg8: f32):
    %3 = arith.index_cast %arg5 : i32 to index
    %4 = arith.index_cast %arg6 : i32 to index
    %7 = tensor.extract %arg0[%3, %4] : tensor<3x3xf32>
    linalg.yield %7 : f32
  } -> tensor<4x7x3x2xf32>
  return %2 : tensor<4x7x3x2xf32>
}
// CHECK-LABEL: func.func @vectorize_nd_tensor_extract_index_from_tensor
// CHECK-SAME: %[[ARG0:.*]]: tensor<3x3xf32>
// CHECK-SAME: %[[ARG1:arg1]]: tensor<4x3xi32>
// CHECK-SAME: %[[ARG2:arg2]]: tensor<4x3xi32>
// CHECK-SAME: %[[ARG3:.*]]: tensor<4x7x2xf32>
// CHECK-SAME: %[[ARG4:.*]]: tensor<4x7x3x2xf32>
// CHECK-DAG: %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG: %[[PV:.*]] = ub.poison : i32
// CHECK-DAG: %[[CST:.*]] = arith.constant dense<3> : vector<4x3xindex>
// CHECK-DAG: %[[CST_1:.*]] = arith.constant dense<true> : vector<4x7x3x2xi1>
// CHECK-DAG: %[[PASSTHRU:.*]] = arith.constant dense<0.000000e+00> : vector<4x7x3x2xf32>
// CHECK:    %[[V0:.*]] = vector.transfer_read %[[ARG1]][%[[C0]], %[[C0]]], %[[PV]] {in_bounds = [true, true]} : tensor<4x3xi32>, vector<4x3xi32>
// CHECK:    %[[V1:.*]] = vector.transfer_read %[[ARG2]][%[[C0]], %[[C0]]], %[[PV]] {in_bounds = [true, true]} : tensor<4x3xi32>, vector<4x3xi32>
// CHECK:    %[[CAST:.*]] = arith.index_cast %[[V0]] : vector<4x3xi32> to vector<4x3xindex>
// CHECK:    %[[CAST_1:.*]] = arith.index_cast %[[V1]] : vector<4x3xi32> to vector<4x3xindex>
// CHECK:    %[[MULI:.*]] = arith.muli %[[CAST]], %[[CST]] : vector<4x3xindex>
// CHECK:    %[[ADDI:.*]] = arith.addi %[[CAST_1]], %[[MULI]] : vector<4x3xindex>
// CHECK:    %[[B:.*]] = vector.broadcast %[[ADDI]] : vector<4x3xindex> to vector<7x2x4x3xindex>
// CHECK:    %[[T:.*]] = vector.transpose %[[B]], [2, 0, 3, 1] : vector<7x2x4x3xindex> to vector<4x7x3x2xindex>
// CHECK:    %[[GATHER:.*]] = vector.gather %[[ARG0]][%[[C0]], %[[C0]]] [%[[T]]], %[[CST_1]], %[[PASSTHRU]] : tensor<3x3xf32>, vector<4x7x3x2xindex>, vector<4x7x3x2xi1>, vector<4x7x3x2xf32> into vector<4x7x3x2xf32>
// CHECK:    vector.transfer_write %[[GATHER]], %[[ARG4]][%[[C0]], %[[C0]], %[[C0]], %[[C0]]] {in_bounds = [true, true, true, true]} : vector<4x7x3x2xf32>, tensor<4x7x3x2xf32>

// -----

#map = affine_map<(d0, d1) -> (d0, d1)>
#map1 = affine_map<(d0, d1, d2) -> (d0 + d1 + d2)>
func.func @vectorize_nd_tensor_extract_load_1d_column_vector_using_gather_load(%arg0: tensor<8x128x768xf32>, %arg1 : index) -> tensor<8x1xf32> {
  %c0 = arith.constant 0 : index
  %0 = tensor.empty() : tensor<8x1xf32>
  %1 = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel", "parallel"]
  } outs(%0 : tensor<8x1xf32>) {
  ^bb0(%arg5: f32):
      %2 = linalg.index 0 : index
      %3 = linalg.index 1 : index
      %4 = affine.apply #map1(%arg1, %3, %arg1)
    %extracted = tensor.extract %arg0[%2, %c0, %4] : tensor<8x128x768xf32>
    linalg.yield %extracted : f32
  } -> tensor<8x1xf32>
  return %1 : tensor<8x1xf32>
}

// CHECK-LABEL: func.func @vectorize_nd_tensor_extract_load_1d_column_vector_using_gather_load
// CHECK-SAME: %[[ARG0:.*]]: tensor<8x128x768xf32>
// CHECK-SAME: %[[ARG1:.*]]: index
// CHECK-DAG: %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG: %[[PASSTHRU:.*]] = arith.constant dense<0.000000e+00> : vector<8x1xf32>
// CHECK-DAG: %[[CST_0:.*]] = arith.constant dense<true> : vector<8x1xi1>
// CHECK-DAG: %[[CST_1:.*]] = arith.constant dense<[0, 98304, 196608, 294912, 393216, 491520, 589824, 688128]> : vector<8xindex>
// CHECK: %[[EMPTY:.*]] = tensor.empty() : tensor<8x1xf32>
// CHECK: %[[ADDI_ARG1:.*]] = arith.addi %[[ARG1]], %[[ARG1]] : index
// CHECK: %[[B1:.*]] = vector.broadcast %[[CST_1]] : vector<8xindex> to vector<1x8xindex>
// CHECK: %[[T:.*]] = vector.transpose %[[B1]], [1, 0] : vector<1x8xindex> to vector<8x1xindex>
// CHECK: %[[B2:.*]] = vector.broadcast %[[ADDI_ARG1]] : index to vector<8x1xindex>
// CHECK: %[[ADDI:.*]] = arith.addi %[[B2]], %[[T]] : vector<8x1xindex>
// CHECK: %[[GATHER:.*]] = vector.gather %[[ARG0]][%[[C0]], %[[C0]], %[[C0]]] [%[[ADDI]]], %[[CST_0]], %[[PASSTHRU]] : tensor<8x128x768xf32>, vector<8x1xindex>, vector<8x1xi1>, vector<8x1xf32> into vector<8x1xf32>
// CHECK: vector.transfer_write %[[GATHER]], %[[EMPTY]][%[[C0]], %[[C0]]] {in_bounds = [true, true]} : vector<8x1xf32>, tensor<8x1xf32>

// -----

// Reading a 1D column vector (hence a candidate for a contiguous load), but given
// %1, it's a gather load.

#map = affine_map<(d0, d1) -> (d0, d1)>
func.func @index_from_output_column_vector_gather_load(%src: tensor<8x128xf32>) -> tensor<8x1xf32> {
  %c0 = arith.constant 0 : index
  %0 = tensor.empty() : tensor<8x1xf32>
  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel", "parallel"]
  } outs(%0 : tensor<8x1xf32>) {
  ^bb0(%arg1: f32):
      %1 = linalg.index 0 : index
    %extracted = tensor.extract %src[%1, %c0] : tensor<8x128xf32>
      linalg.yield %extracted : f32
  } -> tensor<8x1xf32>
  return %res : tensor<8x1xf32>
}

// CHECK-LABEL:   func.func @index_from_output_column_vector_gather_load(
// CHECK-SAME:      %[[SRC:.*]]: tensor<8x128xf32>) -> tensor<8x1xf32> {
// CHECK:           %[[IDX_VEC:.*]] = arith.constant dense<[0, 128, 256, 384, 512, 640, 768, 896]> : vector<8xindex>
// CHECK:           %[[C0:.*]] = arith.constant 0 : index
// CHECK:           %[[PASS_THRU:.*]] = arith.constant dense<0.000000e+00> : vector<8x1xf32>
// CHECK:           %[[MASK:.*]] = arith.constant dense<true> : vector<8x1xi1>
// CHECK:           %[[OUT:.*]] = tensor.empty() : tensor<8x1xf32>
// CHECK:           %[[B:.*]] = vector.broadcast %[[IDX_VEC]] : vector<8xindex> to vector<1x8xindex>
// CHECK:           %[[TR:.*]] = vector.transpose %[[B]], [1, 0] : vector<1x8xindex> to vector<8x1xindex>
// CHECK:           %[[GATHER:.*]] = vector.gather %[[SRC]]{{\[}}%[[C0]], %[[C0]]] {{\[}}%[[TR]]], %[[MASK]], %[[PASS_THRU]] : tensor<8x128xf32>, vector<8x1xindex>, vector<8x1xi1>, vector<8x1xf32> into vector<8x1xf32>
// CHECK:           %[[RES:.*]] = vector.transfer_write %[[GATHER]], %[[OUT]]{{\[}}%[[C0]], %[[C0]]] {in_bounds = [true, true]} : vector<8x1xf32>, tensor<8x1xf32>
// CHECK:           return %[[RES]] : tensor<8x1xf32>

// -----

// Same as above, but the access indices have been swapped and hence this _is_
// a contiguous load. Currently not supported and lowered as vector.gather
// instead.
// TODO: Make sure that this is lowered as a contiguous load.

#map = affine_map<(d0, d1) -> (d0, d1)>
func.func @index_from_output_column_vector_contiguous_load(%src: tensor<8x128xf32>) -> tensor<8x1xf32> {
  %c0 = arith.constant 0 : index
  %0 = tensor.empty() : tensor<8x1xf32>
  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel", "parallel"]
  } outs(%0 : tensor<8x1xf32>) {
  ^bb0(%arg1: f32):
      %1 = linalg.index 0 : index
    %extracted = tensor.extract %src[%c0, %1] : tensor<8x128xf32>
      linalg.yield %extracted : f32
  } -> tensor<8x1xf32>
  return %res : tensor<8x1xf32>
}

// CHECK-LABEL:   func.func @index_from_output_column_vector_contiguous_load(
// CHECK-SAME:      %[[SRC:.*]]: tensor<8x128xf32>) -> tensor<8x1xf32> {
// CHECK:           %[[C0:.*]] = arith.constant 0 : index
// CHECK:           %[[PASS_THRU:.*]] = arith.constant dense<0.000000e+00> : vector<8x1xf32>
// CHECK:           %[[MASK:.*]] = arith.constant dense<true> : vector<8x1xi1>
// CHECK:           %[[IDX_VEC:.*]] = arith.constant dense<[0, 1, 2, 3, 4, 5, 6, 7]> : vector<8xindex>
// CHECK:           %[[OUT:.*]] = tensor.empty() : tensor<8x1xf32>
// CHECK:           %[[B:.*]] = vector.broadcast %[[IDX_VEC]] : vector<8xindex> to vector<1x8xindex>
// CHECK:           %[[TR:.*]] = vector.transpose %[[B]], [1, 0] : vector<1x8xindex> to vector<8x1xindex>
// CHECK:           %[[GATHER:.*]] = vector.gather %[[SRC]]{{\[}}%[[C0]], %[[C0]]] {{\[}}%[[TR]]], %[[MASK]], %[[PASS_THRU]] : tensor<8x128xf32>, vector<8x1xindex>, vector<8x1xi1>, vector<8x1xf32> into vector<8x1xf32>
// CHECK:           %[[RES:.*]] = vector.transfer_write %[[GATHER]], %[[OUT]]{{\[}}%[[C0]], %[[C0]]] {in_bounds = [true, true]} : vector<8x1xf32>, tensor<8x1xf32>
// CHECK:           return %[[RES]] : tensor<8x1xf32>

// -----

#map = affine_map<(d0) -> (d0)>
func.func @vectorize_nd_tensor_extract_contiguous_and_gather(%arg0: tensor<6xf32>, %arg1: tensor<5xi32>) -> tensor<5xf32> {
 %c5 = arith.constant 5 : index
 %c0 = arith.constant 0 : index
 %0 = tensor.empty() : tensor<5xf32>
 %1 = linalg.generic {indexing_maps = [#map], iterator_types = ["parallel"]} outs(%0 : tensor<5xf32>) {
 ^bb0(%out: f32):
   %2 = linalg.index 0 : index
   %extracted = tensor.extract %arg1[%2] : tensor<5xi32>
   %3 = arith.index_cast %extracted : i32 to index
   %4 = arith.maxsi %3, %c0 : index
   %5 = arith.minsi %4, %c5 : index
   %extracted_0 = tensor.extract %arg0[%5] : tensor<6xf32>
   linalg.yield %extracted_0 : f32
 } -> tensor<5xf32>
 return %1 : tensor<5xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_contiguous_and_gather(
// CHECK-SAME:                    %[[VAL_0:.*]]: tensor<6xf32>
// CHECK-SAME:                    %[[VAL_1:.*]]: tensor<5xi32>
// CHECK-DAG:       %[[VAL_2:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[VAL_3:.*]] = ub.poison : i32
// CHECK-DAG:       %[[VAL_4:.*]] = arith.constant dense<0> : vector<5xindex>
// CHECK-DAG:       %[[VAL_5:.*]] = arith.constant dense<5> : vector<5xindex>
// CHECK-DAG:       %[[VAL_6:.*]] = arith.constant dense<true> : vector<5xi1>
// CHECK-DAG:       %[[VAL_7:.*]] = arith.constant dense<0.000000e+00> : vector<5xf32>
// CHECK:           %[[VAL_8:.*]] = tensor.empty() : tensor<5xf32>
// CHECK:           %[[VAL_9:.*]] = vector.transfer_read %[[VAL_1]]{{\[}}%{{.*}}], %[[VAL_3]] {in_bounds = [true]} : tensor<5xi32>, vector<5xi32>
// CHECK:           %[[VAL_10:.*]] = arith.index_cast %[[VAL_9]] : vector<5xi32> to vector<5xindex>
// CHECK:           %[[VAL_11:.*]] = arith.maxsi %[[VAL_10]], %[[VAL_4]] : vector<5xindex>
// CHECK:           %[[VAL_12:.*]] = arith.minsi %[[VAL_11]], %[[VAL_5]] : vector<5xindex>
// CHECK:           %[[VAL_13:.*]] = vector.gather %[[VAL_0]]{{\[}}%[[VAL_2]]] {{\[}}%[[VAL_12]]], %[[VAL_6]], %[[VAL_7]] : tensor<6xf32>, vector<5xindex>, vector<5xi1>, vector<5xf32> into vector<5xf32>
// CHECK:           %[[VAL_14:.*]] = vector.transfer_write %[[VAL_13]], %[[VAL_8]]{{\[}}%[[VAL_2]]] {in_bounds = [true]} : vector<5xf32>, tensor<5xf32>
// CHECK:           return %[[VAL_14]] : tensor<5xf32>

// The vectorizer converts `affine.apply` so that the subsequent Ops can be vectorised based on the converted ops. Gather load.
func.func @vectorize_nd_tensor_extract_with_affine_apply_gather(%6: tensor<80x16xf32>, %arg0: index, %extracted_slice : tensor<1x4xf32>) -> tensor<1x4xf32> {
  %c16 = arith.constant 16 : index
  %1 = linalg.generic {
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>],
    iterator_types = ["parallel", "parallel"]
  } outs(%extracted_slice : tensor<1x4xf32>) {
  ^bb0(%out: f32):
    %2 = linalg.index 1 : index
    %3 = affine.apply affine_map<(d0, d1) -> (d0 + d1)>(%2, %arg0)
    %extracted = tensor.extract %6[%3, %c16] : tensor<80x16xf32>
    linalg.yield %extracted : f32
  } -> tensor<1x4xf32>
  return %1 : tensor<1x4xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_with_affine_apply_gather(
// CHECK-SAME:                                                                    %[[VAL_0:.*]]: tensor<80x16xf32>,
// CHECK-SAME:                                                                    %[[VAL_1:.*]]: index,
// CHECK-SAME:                                                                    %[[VAL_2:.*]]: tensor<1x4xf32>) -> tensor<1x4xf32> {
// CHECK-DAG:       %[[VAL_3:.*]] = arith.constant dense<[0, 1, 2, 3]> : vector<4xindex>
// CHECK-DAG:       %[[VAL_4:.*]] = arith.constant dense<true> : vector<1x4xi1>
// CHECK-DAG:       %[[VAL_5:.*]] = arith.constant dense<0.000000e+00> : vector<1x4xf32>
// CHECK-DAG:       %[[VAL_6:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[VAL_7:.*]] = arith.constant dense<16> : vector<4xindex>
// CHECK:           %[[VAL_8:.*]] = vector.broadcast %[[VAL_1]] : index to vector<4xindex>
// CHECK:           %[[VAL_9:.*]] = arith.addi %[[VAL_8]], %[[VAL_3]] : vector<4xindex>
// CHECK:           %[[VAL_10:.*]] = arith.muli %[[VAL_9]], %[[VAL_7]] : vector<4xindex>
// CHECK:           %[[VAL_11:.*]] = arith.addi %[[VAL_10]], %[[VAL_7]] : vector<4xindex>
// CHECK:           %[[VAL_12:.*]] = vector.broadcast %[[VAL_11]] : vector<4xindex> to vector<1x4xindex>
// CHECK:           %[[VAL_13:.*]] = vector.gather %[[VAL_0]]{{\[}}%[[VAL_6]], %[[VAL_6]]] {{\[}}%[[VAL_12]]], %[[VAL_4]], %[[VAL_5]] : tensor<80x16xf32>, vector<1x4xindex>, vector<1x4xi1>, vector<1x4xf32> into vector<1x4xf32>
// CHECK:           %[[VAL_14:.*]] = vector.transfer_write %[[VAL_13]], %[[VAL_2]]{{\[}}%[[VAL_6]], %[[VAL_6]]] {in_bounds = [true, true]} : vector<1x4xf32>, tensor<1x4xf32>
// CHECK:           return %[[VAL_14]] : tensor<1x4xf32>
// CHECK:         }

// Make sure that non-linear arithmetic operations (e.g. arith.maxsi) are allowed when calculating indices for load operations. Gather load.
func.func @vectorize_nd_tensor_extract_with_maxsi_gather(%arg0: tensor<80x16xf32>, %extracted_slice : tensor<1x4xf32>) -> tensor<1x4xf32> {
  %c79 = arith.constant 79 : index
  %1 = linalg.generic {
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>],
    iterator_types = ["parallel", "parallel"]
  } outs(%extracted_slice : tensor<1x4xf32>) {
  ^bb0(%out: f32):
    %2 = linalg.index 1 : index
    %3 = arith.maxsi %2, %c79 : index
    %extracted = tensor.extract %arg0[%3, %2] : tensor<80x16xf32>
    linalg.yield %extracted : f32
  } -> tensor<1x4xf32>
  return %1 : tensor<1x4xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_with_maxsi_gather(
// CHECK-SAME:                                                             %[[VAL_0:.*]]: tensor<80x16xf32>,
// CHECK-SAME:                                                             %[[VAL_1:.*]]: tensor<1x4xf32>) -> tensor<1x4xf32> {
// CHECK-DAG:       %[[VAL_2:.*]] = arith.constant dense<[1264, 1265, 1266, 1267]> : vector<4xindex>
// CHECK-DAG:       %[[VAL_4:.*]] = arith.constant dense<true> : vector<1x4xi1>
// CHECK-DAG:       %[[VAL_5:.*]] = arith.constant dense<0.000000e+00> : vector<1x4xf32>
// CHECK-DAG:       %[[VAL_6:.*]] = arith.constant 0 : index
// CHECK:           %[[VAL_7:.*]] = vector.broadcast %[[VAL_2]] : vector<4xindex> to vector<1x4xindex>
// CHECK:           %[[VAL_9:.*]] = vector.gather %[[VAL_0]]{{\[}}%[[VAL_6]], %[[VAL_6]]] {{\[}}%[[VAL_7]]], %[[VAL_4]], %[[VAL_5]] : tensor<80x16xf32>, vector<1x4xindex>, vector<1x4xi1>, vector<1x4xf32> into vector<1x4xf32>
// CHECK:           %[[VAL_10:.*]] = vector.transfer_write %[[VAL_9]], %[[VAL_1]]{{\[}}%[[VAL_6]], %[[VAL_6]]] {in_bounds = [true, true]} : vector<1x4xf32>, tensor<1x4xf32>
// CHECK:           return %[[VAL_10]] : tensor<1x4xf32>
// CHECK:         }

// -----

// The vectorizer assumes it's a gather load whenever using a block argument to calculate an index.
#map = affine_map<(d0) -> (d0)>
func.func @vectorize_nd_tensor_extract_block_arg(%arg0: tensor<5x6xf32>, %arg1: tensor<5xindex>) -> tensor<5xf32> {
 %0 = tensor.empty() : tensor<5xf32>
 %1 = linalg.generic {indexing_maps = [#map, #map], iterator_types = ["parallel"]} ins(%arg1: tensor<5xindex>) outs(%0 : tensor<5xf32>) {
 ^bb0(%in: index, %out: f32):
   %2 = linalg.index 0 : index
   %extracted_0 = tensor.extract %arg0[%in, %2] : tensor<5x6xf32>
   linalg.yield %extracted_0 : f32
 } -> tensor<5xf32>
 return %1 : tensor<5xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_block_arg(
// CHECK-SAME:                                                     %[[VAL_0:.*]]: tensor<5x6xf32>,
// CHECK-SAME:                                                     %[[VAL_1:.*]]: tensor<5xindex>) -> tensor<5xf32> {
// CHECK-DAG:       %[[PAD:.*]] = ub.poison : index
// CHECK-DAG:       %[[VAL_2:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[VAL_3:.*]] = arith.constant dense<[0, 1, 2, 3, 4]> : vector<5xindex>
// CHECK-DAG:       %[[VAL_4:.*]] = arith.constant dense<true> : vector<5xi1>
// CHECK-DAG:       %[[VAL_5:.*]] = arith.constant dense<0.000000e+00> : vector<5xf32>
// CHECK-DAG:       %[[VAL_6:.*]] = arith.constant dense<6> : vector<5xindex>
// CHECK:           %[[VAL_7:.*]] = tensor.empty() : tensor<5xf32>
// CHECK:           %[[VAL_8:.*]] = vector.transfer_read %[[VAL_1]]{{\[}}%[[VAL_2]]], %[[PAD]] {in_bounds = [true]} : tensor<5xindex>, vector<5xindex>
// CHECK:           %[[VAL_9:.*]] = arith.muli %[[VAL_8]], %[[VAL_6]] : vector<5xindex>
// CHECK:           %[[VAL_10:.*]] = arith.addi %[[VAL_9]], %[[VAL_3]] : vector<5xindex>
// CHECK:           %[[VAL_11:.*]] = vector.gather %[[VAL_0]]{{\[}}%[[VAL_2]], %[[VAL_2]]] {{\[}}%[[VAL_10]]], %[[VAL_4]], %[[VAL_5]] : tensor<5x6xf32>, vector<5xindex>, vector<5xi1>, vector<5xf32> into vector<5xf32>
// CHECK:           %[[VAL_12:.*]] = vector.transfer_write %[[VAL_11]], %[[VAL_7]]{{\[}}%[[VAL_2]]] {in_bounds = [true]} : vector<5xf32>, tensor<5xf32>
// CHECK:           return %[[VAL_12]] : tensor<5xf32>
// CHECK:         }

// -----

#map = affine_map<(d0, d1, d2) -> (d0, d1, d2)>
#map1 = affine_map<(d0, d1, d2) -> (d0 + d1 + d2)>
func.func @vectorize_reverse_like_tensor_extract(%arg0: tensor<1x2x3xf32>, %arg1: tensor<1x1x3xf32>, %arg2: index) -> tensor<1x1x3xf32> {
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %c2 = arith.constant 2 : index
  %0 = linalg.generic {indexing_maps = [#map], iterator_types = ["parallel", "parallel", "parallel"]} outs(%arg1 : tensor<1x1x3xf32>) {
  ^bb0(%out: f32):
    %1 = linalg.index 1 : index
    %2 = linalg.index 0 : index
    %3 = affine.apply #map1(%1, %2, %arg2)
    %4 = linalg.index 2 : index
    %5 = arith.subi %c2, %4 : index
    %extracted = tensor.extract %arg0[%c0, %3, %5] : tensor<1x2x3xf32>
    linalg.yield %extracted : f32
  } -> tensor<1x1x3xf32>
  return %0 : tensor<1x1x3xf32>
}
// CHECK-LABEL: func.func @vectorize_reverse_like_tensor_extract
// CHECK-SAME:    %[[ARG0:[0-9a-zA-Z]*]]
// CHECK-SAME:    %[[ARG1:[0-9a-zA-Z]*]]
// CHECK-SAME:    %[[ARG2:[0-9a-zA-Z]*]]
// CHECK-DAG:    %[[C3:.+]] = arith.constant 3 : index
// CHECK-DAG:    %[[C0:.+]] = arith.constant 0 : index
// CHECK-DAG:    %[[MASK:.*]] = arith.constant dense<true> : vector<1x1x3xi1>
// CHECK-DAG:    %[[PASSTHRU:.*]] = arith.constant dense<0.000000e+00> : vector<1x1x3xf32>
// CHECK-DAG:    %[[INIT_IDX:.+]] = arith.constant dense<[2, 1, 0]> : vector<3xindex>
// CHECK:        %[[T0:.+]] = arith.muli %[[ARG2]], %[[C3]] : index
// CHECK:        %[[T1:.+]] = vector.broadcast %[[T0]] : index to vector<1x1x3xindex>
// CHECK:        %[[T2:.+]] = vector.broadcast %[[INIT_IDX]]
// CHECK:        %[[T3:.+]] = arith.addi %[[T2]], %[[T1]]
// CHECK:        %[[GATHER:.*]] = vector.gather %[[ARG0]][%[[C0]], %[[C0]], %[[C0]]] [%[[T3]]], %[[MASK]], %[[PASSTHRU]]
// CHECK:        vector.transfer_write %[[GATHER]]

//===----------------------------------------------------------------------===//
// Scalar load + broadcast
//===----------------------------------------------------------------------===//

// -----

#map = affine_map<(d0, d1, d2) -> (d0, d1, d2)>
func.func @vectorize_nd_tensor_extract_scalar_broadcast(%src: tensor<3x3xf32>, %init: tensor<1x1x3xf32>) -> tensor<1x1x3xf32> {
  %c0 = arith.constant 1 : index
  %c1 = arith.constant 2 : index

  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel", "parallel", "parallel"]
  } outs(%init : tensor<1x1x3xf32>) {
  ^bb0(%arg4: f32):
    %1 = tensor.extract %src[%c0, %c1] : tensor<3x3xf32>
    linalg.yield %1 : f32
  } -> tensor<1x1x3xf32>

  return %res : tensor<1x1x3xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_scalar_broadcast(
// CHECK-SAME:      %[[SRC:.*]]: tensor<3x3xf32>,
// CHECK-SAME:      %[[INIT:.*]]: tensor<1x1x3xf32>) -> tensor<1x1x3xf32> {
// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[C1:.*]] = arith.constant 1 : index
// CHECK-DAG:       %[[C2:.*]] = arith.constant 2 : index
// CHECK-DAG:       %[[PAD:.*]] = ub.poison : f32
// CHECK:           %[[READ:.*]] = vector.transfer_read %[[SRC]][%[[C1]], %[[C2]]], %[[PAD]] : tensor<3x3xf32>, vector<f32>
// CHECK:           %[[READ_BCAST:.*]] = vector.broadcast %[[READ]] : vector<f32> to vector<1x1x3xf32>
// CHECK:           vector.transfer_write %[[READ_BCAST]], %[[INIT]][%[[C0]], %[[C0]], %[[C0]]] {in_bounds = [true, true, true]} : vector<1x1x3xf32>, tensor<1x1x3xf32>

// -----

#map = affine_map<() -> ()>
func.func @extract_scalar_from_0d_into_0d(%src: tensor<f32>, %init: tensor<f32>) -> tensor<f32> {
  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = []
  } outs(%init : tensor<f32>) {
  ^bb0(%in: f32):
    %1 = tensor.extract %src[] : tensor<f32>
    linalg.yield %1 : f32
  } -> tensor<f32>

  return %res : tensor<f32>
}

// CHECK-LABEL:   func.func @extract_scalar_from_0d_into_0d(
// CHECK-SAME:      %[[SRC:.*]]: tensor<f32>,
// CHECK-SAME:      %[[INIT:.*]]: tensor<f32>) -> tensor<f32> {
// CHECK:           %[[PAD:.*]] = ub.poison : f32
// CHECK:           %[[READ:.*]] = vector.transfer_read %[[SRC]][], %[[PAD]] : tensor<f32>, vector<f32>
// CHECK:           vector.transfer_write %[[READ]], %[[INIT]][] : vector<f32>, tensor<f32>

// -----

#map = affine_map<(n) -> (n)>
func.func @extract_scalar_from_0d_into_1d(%src: tensor<f32>, %init: tensor<1xf32>) -> tensor<1xf32> {
  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel"]
  } outs(%init : tensor<1xf32>) {
  ^bb0(%in: f32):
    %1 = tensor.extract %src[] : tensor<f32>
    linalg.yield %1 : f32
  } -> tensor<1xf32>

  return %res : tensor<1xf32>
}
// CHECK-LABEL:   func.func @extract_scalar_from_0d_into_1d(
// CHECK-SAME:      %[[SRC:.*]]: tensor<f32>,
// CHECK-SAME:      %[[INIT:.*]]: tensor<1xf32>) -> tensor<1xf32> {
// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[PAD:.*]] = ub.poison : f32
// CHECK:           %[[READ:.*]] = vector.transfer_read %[[SRC]][], %[[PAD]] : tensor<f32>, vector<f32>
// CHECK:           %[[READ_BCAST:.*]] = vector.broadcast %[[READ]] : vector<f32> to vector<1xf32>
// CHECK:           vector.transfer_write %[[READ_BCAST]], %[[INIT]][%[[C0]]] {in_bounds = [true]} : vector<1xf32>, tensor<1xf32>

// -----

#map1 = affine_map<(d0, d1, d2) -> (d0, d1, d2)>
func.func @vectorize_0d_tensor_extract(%src: tensor<f32>, %init: tensor<1x1x3xf32>) -> tensor<1x1x3xf32> {
  %res = linalg.generic {
    indexing_maps = [#map1],
    iterator_types = ["parallel", "parallel", "parallel"]
  } outs(%init : tensor<1x1x3xf32>) {
  ^bb0(%arg4: f32):
    %1 = tensor.extract %src[] : tensor<f32>
    linalg.yield %1 : f32
  } -> tensor<1x1x3xf32>
  return %res : tensor<1x1x3xf32>
}

// CHECK-LABEL:   func.func @vectorize_0d_tensor_extract(
// CHECK-SAME:     %[[SRC:.*]]: tensor<f32>
// CHECK:           %[[READ:.*]] = vector.transfer_read %[[SRC]][], %{{.+}} : tensor<f32>
// CHECK:           vector.broadcast %[[READ]] : vector<f32> to vector<1x1x3xf32>

// -----

func.func @scalar_read_with_broadcast_from_column_tensor(%init: tensor<1x1x4xi32>) -> tensor<1x1x4xi32> {
  %c4 = arith.constant 4 : index
  %c0 = arith.constant 0 : index
  %src = arith.constant dense<[[0], [1], [2], [3], [4], [5], [6], [7], [8], [9], [10], [11], [12], [13], [14]]> : tensor<15x1xi32>

  %res = linalg.generic {
    indexing_maps = [affine_map<(d0, d1, d2) -> (d0, d1, d2)>],
    iterator_types = ["parallel", "parallel", "parallel"]
  } outs(%init : tensor<1x1x4xi32>) {

    ^bb0(%out: i32):
      %idx = linalg.index 0 : index
      %extracted = tensor.extract %src[%idx, %c0] : tensor<15x1xi32>
      linalg.yield %extracted : i32
  } -> tensor<1x1x4xi32>

  return %res : tensor<1x1x4xi32>
}

// CHECK-LABEL:   func.func @scalar_read_with_broadcast_from_column_tensor
// CHECK-SAME:      %[[INIT:.*]]: tensor<1x1x4xi32>) -> tensor<1x1x4xi32> {
// CHECK-DAG:       %[[PAD:.*]] = ub.poison : i32
// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[SRC:.*]] = arith.constant dense<{{\[\[}}0], [1], [2], [3], [4], [5], [6], [7], [8], [9], [10], [11], [12], [13], [14]]> : tensor<15x1xi32>
// CHECK:           %[[READ:.*]] = vector.transfer_read %[[SRC]]{{\[}}%[[C0]], %[[C0]]], %[[PAD]] : tensor<15x1xi32>, vector<i32>
// CHECK:           %[[READ_BCAST:.*]] = vector.broadcast %[[READ]] : vector<i32> to vector<1x1x4xi32>
// CHECK:           %[[RES:.*]] = vector.transfer_write %[[READ_BCAST]], %[[INIT]][%[[C0]], %[[C0]], %[[C0]]] {in_bounds = [true, true, true]} : vector<1x1x4xi32>, tensor<1x1x4xi32>

// -----

// TODO: Currently this fails to vectorise when the indices are non-constant.

#map = affine_map<(d0, d1, d2) -> (d0, d1, d2)>
func.func @vectorize_nd_tensor_extract_transfer_read_basic_column(
    %src: tensor<3x3x3xf32>,
    %init: tensor<3x1x1xf32>) -> tensor<3x1x1xf32> {

  %c0 = arith.constant 0 : index

  %res = linalg.generic {
    indexing_maps = [#map],
    iterator_types = ["parallel", "parallel", "parallel"]
  } outs(%init : tensor<3x1x1xf32>) {
  ^bb0(%out: f32):
    %1 = tensor.extract %src[%c0, %c0, %c0] : tensor<3x3x3xf32>
    linalg.yield %1 : f32
  } -> tensor<3x1x1xf32>

  return %res : tensor<3x1x1xf32>
}

// CHECK-LABEL:   func.func @vectorize_nd_tensor_extract_transfer_read_basic_column(
// CHECK-SAME:      %[[SRC:.*]]: tensor<3x3x3xf32>,
// CHECK-SAME:      %[[INIT:.*]]: tensor<3x1x1xf32>)
// CHECK-DAG:       %[[C0:.*]] = arith.constant 0 : index
// CHECK-DAG:       %[[PV:.*]] = ub.poison : f32
// CHECK:           %[[READ:.*]] = vector.transfer_read %[[SRC]][%[[C0]], %[[C0]], %[[C0]]], %[[PV]] : tensor<3x3x3xf32>, vector<f32>
// CHECK:           %[[READ_BCAST:.*]] = vector.broadcast %[[READ]] : vector<f32> to vector<3x1x1xf32>
// CHECK:           vector.transfer_write %[[READ_BCAST]], %[[INIT]]{{\[}}%[[C0]], %[[C0]], %[[C0]]] {in_bounds = [true, true, true]} : vector<3x1x1xf32>, tensor<3x1x1xf32>
