/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_PARTITION_MODELS_H_
#define VPX_VP9_ENCODER_VP9_PARTITION_MODELS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NN_MAX_HIDDEN_LAYERS 10
#define NN_MAX_NODES_PER_LAYER 128

// Neural net model config. It defines the layout of a neural net model, such as
// the number of inputs/outputs, number of layers, the number of nodes in each
// layer, as well as the weights and bias of each node.
typedef struct {
  int num_inputs;         // Number of input nodes, i.e. features.
  int num_outputs;        // Number of output nodes.
  int num_hidden_layers;  // Number of hidden layers, maximum 10.
  // Number of nodes for each hidden layer.
  int num_hidden_nodes[NN_MAX_HIDDEN_LAYERS];
  // Weight parameters, indexed by layer.
  const float *weights[NN_MAX_HIDDEN_LAYERS + 1];
  // Bias parameters, indexed by layer.
  const float *bias[NN_MAX_HIDDEN_LAYERS + 1];
} NN_CONFIG;

// Partition search breakout model.
#define FEATURES 4
#define Q_CTX 3
#define RESOLUTION_CTX 2
static const float
    vp9_partition_breakout_weights_64[RESOLUTION_CTX][Q_CTX][FEATURES + 1] = {
      {
          {
              -0.016673f,
              -0.001025f,
              -0.000032f,
              0.000833f,
              1.94261885f - 2.1f,
          },
          {
              -0.160867f,
              -0.002101f,
              0.000011f,
              0.002448f,
              1.65738142f - 2.5f,
          },
          {
              -0.628934f,
              -0.011459f,
              -0.000009f,
              0.013833f,
              1.47982645f - 1.6f,
          },
      },
      {
          {
              -0.064309f,
              -0.006121f,
              0.000232f,
              0.005778f,
              0.7989465f - 5.0f,
          },
          {
              -0.314957f,
              -0.009346f,
              -0.000225f,
              0.010072f,
              2.80695581f - 5.5f,
          },
          {
              -0.635535f,
              -0.015135f,
              0.000091f,
              0.015247f,
              2.90381241f - 5.0f,
          },
      },
    };

static const float
    vp9_partition_breakout_weights_32[RESOLUTION_CTX][Q_CTX][FEATURES + 1] = {
      {
          {
              -0.010554f,
              -0.003081f,
              -0.000134f,
              0.004491f,
              1.68445992f - 3.5f,
          },
          {
              -0.051489f,
              -0.007609f,
              0.000016f,
              0.009792f,
              1.28089404f - 2.5f,
          },
          {
              -0.163097f,
              -0.013081f,
              0.000022f,
              0.019006f,
              1.36129403f - 3.2f,
          },
      },
      {
          {
              -0.024629f,
              -0.006492f,
              -0.000254f,
              0.004895f,
              1.27919173f - 4.5f,
          },
          {
              -0.083936f,
              -0.009827f,
              -0.000200f,
              0.010399f,
              2.73731065f - 4.5f,
          },
          {
              -0.279052f,
              -0.013334f,
              0.000289f,
              0.023203f,
              2.43595719f - 3.5f,
          },
      },
    };

static const float
    vp9_partition_breakout_weights_16[RESOLUTION_CTX][Q_CTX][FEATURES + 1] = {
      {
          {
              -0.013154f,
              -0.002404f,
              -0.000977f,
              0.008450f,
              2.57404566f - 5.5f,
          },
          {
              -0.019146f,
              -0.004018f,
              0.000064f,
              0.008187f,
              2.15043926f - 2.5f,
          },
          {
              -0.075755f,
              -0.010858f,
              0.000030f,
              0.024505f,
              2.06848121f - 2.5f,
          },
      },
      {
          {
              -0.007636f,
              -0.002751f,
              -0.000682f,
              0.005968f,
              0.19225763f - 4.5f,
          },
          {
              -0.047306f,
              -0.009113f,
              -0.000518f,
              0.016007f,
              2.61068869f - 4.0f,
          },
          {
              -0.069336f,
              -0.010448f,
              -0.001120f,
              0.023083f,
              1.47591054f - 5.5f,
          },
      },
    };

static const float vp9_partition_breakout_weights_8[RESOLUTION_CTX][Q_CTX]
                                                   [FEATURES + 1] = {
                                                     {
                                                         {
                                                             -0.011807f,
                                                             -0.009873f,
                                                             -0.000931f,
                                                             0.034768f,
                                                             1.32254851f - 2.0f,
                                                         },
                                                         {
                                                             -0.003861f,
                                                             -0.002701f,
                                                             0.000100f,
                                                             0.013876f,
                                                             1.96755111f - 1.5f,
                                                         },
                                                         {
                                                             -0.013522f,
                                                             -0.008677f,
                                                             -0.000562f,
                                                             0.034468f,
                                                             1.53440356f - 1.5f,
                                                         },
                                                     },
                                                     {
                                                         {
                                                             -0.003221f,
                                                             -0.002125f,
                                                             0.000993f,
                                                             0.012768f,
                                                             0.03541421f - 2.0f,
                                                         },
                                                         {
                                                             -0.006069f,
                                                             -0.007335f,
                                                             0.000229f,
                                                             0.026104f,
                                                             0.17135315f - 1.5f,
                                                         },
                                                         {
                                                             -0.039894f,
                                                             -0.011419f,
                                                             0.000070f,
                                                             0.061817f,
                                                             0.6739977f - 1.5f,
                                                         },
                                                     },
                                                   };
#undef FEATURES
#undef Q_CTX
#undef RESOLUTION_CTX

// Rectangular partition search pruning model.
#define FEATURES 8
#define LABELS 4
#define NODES 16
static const float vp9_rect_part_nn_weights_16_layer0[FEATURES * NODES] = {
    -0.432522f,  0.133070f, -0.169187f,  0.768340f,
      0.891228f,  0.554458f,  0.356000f,  0.403621f,
      0.809165f,  0.778214f, -0.520357f,  0.301451f,
     -0.386972f, -0.314402f,  0.021878f,  1.148746f,
     -0.462258f, -0.175524f, -0.344589f, -0.475159f,
     -0.232322f,  0.471147f, -0.489948f,  0.467740f,
     -0.391550f,  0.208601f,  0.054138f,  0.076859f,
     -0.309497f, -0.095927f,  0.225917f,  0.011582f,
     -0.520730f, -0.585497f,  0.174036f,  0.072521f,
      0.120771f, -0.517234f, -0.581908f, -0.034003f,
     -0.694722f, -0.364368f,  0.290584f,  0.038373f,
      0.685654f,  0.394019f,  0.759667f,  1.257502f,
     -0.610516f, -0.185434f,  0.211997f, -0.172458f,
      0.044605f,  0.145316f, -0.182525f, -0.147376f,
      0.578742f,  0.312412f, -0.446135f, -0.389112f,
      0.454033f,  0.260490f,  0.664285f,  0.395856f,
     -0.231827f,  0.215228f,  0.014856f, -0.395462f,
      0.479646f, -0.391445f, -0.357788f,  0.166238f,
     -0.056818f, -0.027783f,  0.060880f, -1.604710f,
      0.531268f,  0.282184f,  0.714944f,  0.093523f,
     -0.218312f, -0.095546f, -0.285621f, -0.190871f,
     -0.448340f, -0.016611f,  0.413913f, -0.286720f,
     -0.158828f, -0.092635f, -0.279551f,  0.166509f,
     -0.088162f,  0.446543f, -0.276830f, -0.065642f,
     -0.176346f, -0.984754f,  0.338738f,  0.403809f,
      0.738065f,  1.154439f,  0.750764f,  0.770959f,
     -0.269403f,  0.295651f, -0.331858f,  0.367144f,
      0.279279f,  0.157419f, -0.348227f, -0.168608f,
     -0.956000f, -0.647136f,  0.250516f,  0.858084f,
      0.809802f,  0.492408f,  0.804841f,  0.282802f,
      0.079395f, -0.291771f, -0.024382f, -1.615880f,
     -0.445166f, -0.407335f, -0.483044f,  0.141126f,
};

static const float vp9_rect_part_nn_bias_16_layer0[NODES] = {
    0.275384f, -0.053745f,  0.000000f,  0.000000f,
     -0.178103f,  0.513965f, -0.161352f,  0.228551f,
      0.000000f,  1.013712f,  0.000000f,  0.000000f,
     -1.144009f, -0.000006f, -0.241727f,  2.048764f,
};

static const float vp9_rect_part_nn_weights_16_layer1[NODES * LABELS] = {
    -1.435278f,  2.204691f, -0.410718f,  0.202708f,
      0.109208f,  1.059142f, -0.306360f,  0.845906f,
      0.489654f, -1.121915f, -0.169133f, -0.003385f,
      0.660590f, -0.018711f,  1.227158f, -2.967504f,
      1.407345f, -1.293243f, -0.386921f,  0.300492f,
      0.338824f, -0.083250f, -0.069454f, -1.001827f,
     -0.327891f,  0.899353f,  0.367397f, -0.118601f,
     -0.171936f, -0.420646f, -0.803319f,  2.029634f,
      0.940268f, -0.664484f,  0.339916f,  0.315944f,
      0.157374f, -0.402482f, -0.491695f,  0.595827f,
      0.015031f,  0.255887f, -0.466327f, -0.212598f,
      0.136485f,  0.033363f, -0.796921f,  1.414304f,
     -0.282185f, -2.673571f, -0.280994f,  0.382658f,
     -0.350902f,  0.227926f,  0.062602f, -1.000199f,
      0.433731f,  1.176439f, -0.163216f, -0.229015f,
     -0.640098f, -0.438852f, -0.947700f,  2.203434f,
};

static const float vp9_rect_part_nn_bias_16_layer1[LABELS] = {
    -0.875510f,  0.982408f,  0.560854f, -0.415209f,
};

static const NN_CONFIG vp9_rect_part_nnconfig_16 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_rect_part_nn_weights_16_layer0,
      vp9_rect_part_nn_weights_16_layer1,
  },
  {
      vp9_rect_part_nn_bias_16_layer0,
      vp9_rect_part_nn_bias_16_layer1,
  },
};

static const float vp9_rect_part_nn_weights_32_layer0[FEATURES * NODES] = {
    -0.147312f, -0.753248f,  0.540206f,  0.661415f,
      0.484117f, -0.341609f,  0.016183f,  0.064177f,
      0.781580f,  0.902232f, -0.505342f,  0.325183f,
     -0.231072f, -0.120107f, -0.076216f,  0.120038f,
      0.403695f, -0.463301f, -0.192158f,  0.407442f,
      0.106633f,  1.072371f, -0.446779f,  0.467353f,
      0.318812f, -0.505996f, -0.008768f, -0.239598f,
      0.085480f,  0.284640f, -0.365045f, -0.048083f,
     -0.112090f, -0.067089f,  0.304138f, -0.228809f,
      0.383651f, -0.196882f,  0.477039f, -0.217978f,
     -0.506931f, -0.125675f,  0.050456f,  1.086598f,
      0.732128f,  0.326941f,  0.103952f,  0.121769f,
     -0.154487f, -0.255514f,  0.030591f, -0.382797f,
     -0.019981f, -0.326570f,  0.149691f, -0.435633f,
     -0.070795f,  0.167691f,  0.251413f, -0.153405f,
      0.160347f,  0.455107f, -0.968580f, -0.575879f,
      0.623115f, -0.069793f, -0.379768f, -0.965807f,
     -0.062057f,  0.071312f,  0.457098f,  0.350372f,
     -0.460659f, -0.985393f,  0.359963f, -0.093677f,
      0.404272f, -0.326896f, -0.277752f,  0.609322f,
     -0.114193f, -0.230701f,  0.089208f,  0.645381f,
      0.494485f,  0.467876f, -0.166187f,  0.251044f,
     -0.394661f,  0.192895f, -0.344777f, -0.041893f,
     -0.111163f,  0.066347f,  0.378158f, -0.455465f,
      0.339839f, -0.418207f, -0.356515f, -0.227536f,
     -0.211091f, -0.122945f,  0.361772f, -0.338095f,
      0.004564f, -0.398510f,  0.060876f, -2.132504f,
     -0.086776f, -0.029166f,  0.039241f,  0.222534f,
     -0.188565f, -0.288792f, -0.160789f, -0.123905f,
      0.397916f, -0.063779f,  0.167210f, -0.445004f,
      0.056889f,  0.207280f,  0.000101f,  0.384507f,
     -1.721239f, -2.036402f, -2.084403f, -2.060483f,
};

static const float vp9_rect_part_nn_bias_32_layer0[NODES] = {
    -0.859251f, -0.109938f,  0.091838f,  0.187817f,
     -0.728265f,  0.253080f,  0.000000f, -0.357195f,
     -0.031290f, -1.373237f, -0.761086f,  0.000000f,
     -0.024504f,  1.765711f,  0.000000f,  1.505390f,
};

static const float vp9_rect_part_nn_weights_32_layer1[NODES * LABELS] = {
    0.680940f,  1.367178f,  0.403075f,  0.029957f,
      0.500917f,  1.407776f, -0.354002f,  0.011667f,
      1.663767f,  0.959155f,  0.428323f, -0.205345f,
     -0.081850f, -3.920103f, -0.243802f, -4.253933f,
     -0.034020f, -1.361057f,  0.128236f, -0.138422f,
     -0.025790f, -0.563518f, -0.148715f, -0.344381f,
     -1.677389f, -0.868332f, -0.063792f,  0.052052f,
      0.359591f,  2.739808f, -0.414304f,  3.036597f,
     -0.075368f, -1.019680f,  0.642501f,  0.209779f,
     -0.374539f, -0.718294f, -0.116616f, -0.043212f,
     -1.787809f, -0.773262f,  0.068734f,  0.508309f,
      0.099334f,  1.802239f, -0.333538f,  2.708645f,
     -0.447682f, -2.355555f, -0.506674f, -0.061028f,
     -0.310305f, -0.375475f,  0.194572f,  0.431788f,
     -0.789624f, -0.031962f,  0.358353f,  0.382937f,
      0.232002f,  2.321813f, -0.037523f,  2.104652f,
};

static const float vp9_rect_part_nn_bias_32_layer1[LABELS] = {
    -0.693383f,  0.773661f,  0.426878f, -0.070619f,
};

static const NN_CONFIG vp9_rect_part_nnconfig_32 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_rect_part_nn_weights_32_layer0,
      vp9_rect_part_nn_weights_32_layer1,
  },
  {
      vp9_rect_part_nn_bias_32_layer0,
      vp9_rect_part_nn_bias_32_layer1,
  },
};
#undef NODES

#define NODES 24
static const float vp9_rect_part_nn_weights_64_layer0[FEATURES * NODES] = {
    0.024671f, -0.220610f, -0.284362f, -0.069556f,
     -0.315700f,  0.187861f,  0.139782f,  0.063110f,
      0.796561f,  0.172868f, -0.662194f, -1.393074f,
      0.085003f,  0.393381f,  0.358477f, -0.187268f,
     -0.370745f,  0.218287f,  0.027271f, -0.254089f,
     -0.048236f, -0.459137f,  0.253171f,  0.122598f,
     -0.550107f, -0.568456f,  0.159866f, -0.246534f,
      0.096384f, -0.255460f,  0.077864f, -0.334837f,
      0.026921f, -0.697252f,  0.345262f,  1.343578f,
      0.815984f,  1.118211f,  1.574016f,  0.578476f,
     -0.285967f, -0.508672f,  0.118137f,  0.037695f,
      1.540510f,  1.256648f,  1.163819f,  1.172027f,
      0.661551f, -0.111980f, -0.434204f, -0.894217f,
      0.570524f,  0.050292f, -0.113680f,  0.000784f,
     -0.211554f, -0.369394f,  0.158306f, -0.512505f,
     -0.238696f,  0.091498f, -0.448490f, -0.491268f,
     -0.353112f, -0.303315f, -0.428438f,  0.127998f,
     -0.406790f, -0.401786f, -0.279888f, -0.384223f,
      0.026100f,  0.041621f, -0.315818f, -0.087888f,
      0.353497f,  0.163123f, -0.380128f, -0.090334f,
     -0.216647f, -0.117849f, -0.173502f,  0.301871f,
      0.070854f,  0.114627f, -0.050545f, -0.160381f,
      0.595294f,  0.492696f, -0.453858f, -1.154139f,
      0.126000f,  0.034550f,  0.456665f, -0.236618f,
     -0.112640f,  0.050759f, -0.449162f,  0.110059f,
      0.147116f,  0.249358f, -0.049894f,  0.063351f,
     -0.004467f,  0.057242f, -0.482015f, -0.174335f,
     -0.085617f, -0.333808f, -0.358440f, -0.069006f,
      0.099260f, -1.243430f, -0.052963f,  0.112088f,
     -2.661115f, -2.445893f, -2.688174f, -2.624232f,
      0.030494f,  0.161311f,  0.012136f,  0.207564f,
     -2.776856f, -2.791940f, -2.623962f, -2.918820f,
      1.231619f, -0.376692f, -0.698078f,  0.110336f,
     -0.285378f,  0.258367f, -0.180159f, -0.376608f,
     -0.034348f, -0.130206f,  0.160020f,  0.852977f,
      0.580573f,  1.450782f,  1.357596f,  0.787382f,
     -0.544004f, -0.014795f,  0.032121f, -0.557696f,
      0.159994f, -0.540908f,  0.180380f, -0.398045f,
      0.705095f,  0.515103f, -0.511521f, -1.271374f,
     -0.231019f,  0.423647f,  0.064907f, -0.255338f,
     -0.877748f, -0.667205f,  0.267847f,  0.135229f,
      0.617844f,  1.349849f,  1.012623f,  0.730506f,
     -0.078571f,  0.058401f,  0.053221f, -2.426146f,
     -0.098808f, -0.138508f, -0.153299f,  0.149116f,
     -0.444243f,  0.301807f,  0.065066f,  0.092929f,
     -0.372784f, -0.095540f,  0.192269f,  0.237894f,
      0.080228f, -0.214074f, -0.011426f, -2.352367f,
     -0.085394f, -0.190361f, -0.001177f,  0.089197f,
};

static const float vp9_rect_part_nn_bias_64_layer0[NODES] = {
    0.000000f, -0.057652f, -0.175413f, -0.175389f,
     -1.084097f, -1.423801f, -0.076307f, -0.193803f,
      0.000000f, -0.066474f, -0.050318f, -0.019832f,
     -0.038814f, -0.144184f,  2.652451f,  2.415006f,
      0.197464f, -0.729842f, -0.173774f,  0.239171f,
      0.486425f,  2.463304f, -0.175279f,  2.352637f,
};

static const float vp9_rect_part_nn_weights_64_layer1[NODES * LABELS] = {
    -0.063237f,  1.925696f, -0.182145f, -0.226687f,
      0.602941f, -0.941140f,  0.814598f, -0.117063f,
      0.282988f,  0.066369f,  0.096951f,  1.049735f,
     -0.188188f, -0.281227f, -4.836746f, -5.047797f,
      0.892358f,  0.417145f, -0.279849f,  1.335945f,
      0.660338f, -2.757938f, -0.115714f, -1.862183f,
     -0.045980f, -1.597624f, -0.586822f, -0.615589f,
     -0.330537f,  1.068496f, -0.167290f,  0.141290f,
     -0.112100f,  0.232761f,  0.252307f, -0.399653f,
      0.353118f,  0.241583f,  2.635241f,  4.026119f,
     -1.137327f, -0.052446f, -0.139814f, -1.104256f,
     -0.759391f,  2.508457f, -0.526297f,  2.095348f,
     -0.444473f, -1.090452f,  0.584122f,  0.468729f,
     -0.368865f,  1.041425f, -1.079504f,  0.348837f,
      0.390091f,  0.416191f,  0.212906f, -0.660255f,
      0.053630f,  0.209476f,  3.595525f,  2.257293f,
     -0.514030f,  0.074203f, -0.375862f, -1.998307f,
     -0.930310f,  1.866686f, -0.247137f,  1.087789f,
      0.100186f,  0.298150f,  0.165265f,  0.050478f,
      0.249167f,  0.371789f, -0.294497f,  0.202954f,
      0.037310f,  0.193159f,  0.161551f,  0.301597f,
      0.299286f,  0.185946f,  0.822976f,  2.066130f,
     -1.724588f,  0.055977f, -0.330747f, -0.067747f,
     -0.475801f,  1.555958f, -0.025808f, -0.081516f,
};

static const float vp9_rect_part_nn_bias_64_layer1[LABELS] = {
    -0.090723f,  0.894968f,  0.844754f, -3.496194f,
};

static const NN_CONFIG vp9_rect_part_nnconfig_64 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_rect_part_nn_weights_64_layer0,
      vp9_rect_part_nn_weights_64_layer1,
  },
  {
      vp9_rect_part_nn_bias_64_layer0,
      vp9_rect_part_nn_bias_64_layer1,
  },
};
#undef FEATURES
#undef LABELS
#undef NODES

#define FEATURES 7
// Partition pruning model(neural nets).
static const float vp9_partition_nn_weights_64x64_layer0[FEATURES * 8] = {
  -3.571348f, 0.014835f,  -3.255393f, -0.098090f, -0.013120f, 0.000221f,
  0.056273f,  0.190179f,  -0.268130f, -1.828242f, -0.010655f, 0.937244f,
  -0.435120f, 0.512125f,  1.610679f,  0.190816f,  -0.799075f, -0.377348f,
  -0.144232f, 0.614383f,  -0.980388f, 1.754150f,  -0.185603f, -0.061854f,
  -0.807172f, 1.240177f,  1.419531f,  -0.438544f, -5.980774f, 0.139045f,
  -0.032359f, -0.068887f, -1.237918f, 0.115706f,  0.003164f,  2.924212f,
  1.246838f,  -0.035833f, 0.810011f,  -0.805894f, 0.010966f,  0.076463f,
  -4.226380f, -2.437764f, -0.010619f, -0.020935f, -0.451494f, 0.300079f,
  -0.168961f, -3.326450f, -2.731094f, 0.002518f,  0.018840f,  -1.656815f,
  0.068039f,  0.010586f,
};

static const float vp9_partition_nn_bias_64x64_layer0[8] = {
  -3.469882f, 0.683989f, 0.194010f,  0.313782f,
  -3.153335f, 2.245849f, -1.946190f, -3.740020f,
};

static const float vp9_partition_nn_weights_64x64_layer1[8] = {
  -8.058566f, 0.108306f, -0.280620f, -0.818823f,
  -6.445117f, 0.865364f, -1.127127f, -8.808660f,
};

static const float vp9_partition_nn_bias_64x64_layer1[1] = {
  6.46909416f,
};

static const NN_CONFIG vp9_partition_nnconfig_64x64 = {
  FEATURES,  // num_inputs
  1,         // num_outputs
  1,         // num_hidden_layers
  {
      8,
  },  // num_hidden_nodes
  {
      vp9_partition_nn_weights_64x64_layer0,
      vp9_partition_nn_weights_64x64_layer1,
  },
  {
      vp9_partition_nn_bias_64x64_layer0,
      vp9_partition_nn_bias_64x64_layer1,
  },
};

static const float vp9_partition_nn_weights_32x32_layer0[FEATURES * 8] = {
  -0.295437f, -4.002648f, -0.205399f, -0.060919f, 0.708037f,  0.027221f,
  -0.039137f, -0.907724f, -3.151662f, 0.007106f,  0.018726f,  -0.534928f,
  0.022744f,  0.000159f,  -1.717189f, -3.229031f, -0.027311f, 0.269863f,
  -0.400747f, -0.394366f, -0.108878f, 0.603027f,  0.455369f,  -0.197170f,
  1.241746f,  -1.347820f, -0.575636f, -0.462879f, -2.296426f, 0.196696f,
  -0.138347f, -0.030754f, -0.200774f, 0.453795f,  0.055625f,  -3.163116f,
  -0.091003f, -0.027028f, -0.042984f, -0.605185f, 0.143240f,  -0.036439f,
  -0.801228f, 0.313409f,  -0.159942f, 0.031267f,  0.886454f,  -1.531644f,
  -0.089655f, 0.037683f,  -0.163441f, -0.130454f, -0.058344f, 0.060011f,
  0.275387f,  1.552226f,
};

static const float vp9_partition_nn_bias_32x32_layer0[8] = {
  -0.838372f, -2.609089f, -0.055763f, 1.329485f,
  -1.297638f, -2.636622f, -0.826909f, 1.012644f,
};

static const float vp9_partition_nn_weights_32x32_layer1[8] = {
  -1.792632f, -7.322353f, -0.683386f, 0.676564f,
  -1.488118f, -7.527719f, 1.240163f,  0.614309f,
};

static const float vp9_partition_nn_bias_32x32_layer1[1] = {
  4.97422546f,
};

static const NN_CONFIG vp9_partition_nnconfig_32x32 = {
  FEATURES,  // num_inputs
  1,         // num_outputs
  1,         // num_hidden_layers
  {
      8,
  },  // num_hidden_nodes
  {
      vp9_partition_nn_weights_32x32_layer0,
      vp9_partition_nn_weights_32x32_layer1,
  },
  {
      vp9_partition_nn_bias_32x32_layer0,
      vp9_partition_nn_bias_32x32_layer1,
  },
};

static const float vp9_partition_nn_weights_16x16_layer0[FEATURES * 8] = {
  -1.717673f, -4.718130f, -0.125725f, -0.183427f, -0.511764f, 0.035328f,
  0.130891f,  -3.096753f, 0.174968f,  -0.188769f, -0.640796f, 1.305661f,
  1.700638f,  -0.073806f, -4.006781f, -1.630999f, -0.064863f, -0.086410f,
  -0.148617f, 0.172733f,  -0.018619f, 2.152595f,  0.778405f,  -0.156455f,
  0.612995f,  -0.467878f, 0.152022f,  -0.236183f, 0.339635f,  -0.087119f,
  -3.196610f, -1.080401f, -0.637704f, -0.059974f, 1.706298f,  -0.793705f,
  -6.399260f, 0.010624f,  -0.064199f, -0.650621f, 0.338087f,  -0.001531f,
  1.023655f,  -3.700272f, -0.055281f, -0.386884f, 0.375504f,  -0.898678f,
  0.281156f,  -0.314611f, 0.863354f,  -0.040582f, -0.145019f, 0.029329f,
  -2.197880f, -0.108733f,
};

static const float vp9_partition_nn_bias_16x16_layer0[8] = {
  0.411516f,  -2.143737f, -3.693192f, 2.123142f,
  -1.356910f, -3.561016f, -0.765045f, -2.417082f,
};

static const float vp9_partition_nn_weights_16x16_layer1[8] = {
  -0.619755f, -2.202391f, -4.337171f, 0.611319f,
  0.377677f,  -4.998723f, -1.052235f, 1.949922f,
};

static const float vp9_partition_nn_bias_16x16_layer1[1] = {
  3.20981717f,
};

static const NN_CONFIG vp9_partition_nnconfig_16x16 = {
  FEATURES,  // num_inputs
  1,         // num_outputs
  1,         // num_hidden_layers
  {
      8,
  },  // num_hidden_nodes
  {
      vp9_partition_nn_weights_16x16_layer0,
      vp9_partition_nn_weights_16x16_layer1,
  },
  {
      vp9_partition_nn_bias_16x16_layer0,
      vp9_partition_nn_bias_16x16_layer1,
  },
};
#undef FEATURES

#if CONFIG_ML_VAR_PARTITION
#define FEATURES 6
static const float vp9_var_part_nn_weights_64_layer0[FEATURES * 8] = {
  -0.249572f, 0.205532f,  -2.175608f, 1.094836f,  -2.986370f, 0.193160f,
  -0.143823f, 0.378511f,  -1.997788f, -2.166866f, -1.930158f, -1.202127f,
  -0.611875f, -0.506422f, -0.432487f, 0.071205f,  0.578172f,  -0.154285f,
  -0.051830f, 0.331681f,  -1.457177f, -2.443546f, -2.000302f, -1.389283f,
  0.372084f,  -0.464917f, 2.265235f,  2.385787f,  2.312722f,  2.127868f,
  -0.403963f, -0.177860f, -0.436751f, -0.560539f, 0.254903f,  0.193976f,
  -0.305611f, 0.256632f,  0.309388f,  -0.437439f, 1.702640f,  -5.007069f,
  -0.323450f, 0.294227f,  1.267193f,  1.056601f,  0.387181f,  -0.191215f,
};

static const float vp9_var_part_nn_bias_64_layer0[8] = {
  -0.044396f, -0.938166f, 0.000000f,  -0.916375f,
  1.242299f,  0.000000f,  -0.405734f, 0.014206f,
};

static const float vp9_var_part_nn_weights_64_layer1[8] = {
  1.635945f,  0.979557f,  0.455315f, 1.197199f,
  -2.251024f, -0.464953f, 1.378676f, -0.111927f,
};

static const float vp9_var_part_nn_bias_64_layer1[1] = {
  -0.37972447f,
};

static const NN_CONFIG vp9_var_part_nnconfig_64 = {
  FEATURES,  // num_inputs
  1,         // num_outputs
  1,         // num_hidden_layers
  {
      8,
  },  // num_hidden_nodes
  {
      vp9_var_part_nn_weights_64_layer0,
      vp9_var_part_nn_weights_64_layer1,
  },
  {
      vp9_var_part_nn_bias_64_layer0,
      vp9_var_part_nn_bias_64_layer1,
  },
};

static const float vp9_var_part_nn_weights_32_layer0[FEATURES * 8] = {
  0.067243f,  -0.083598f, -2.191159f, 2.726434f,  -3.324013f, 3.477977f,
  0.323736f,  -0.510199f, 2.960693f,  2.937661f,  2.888476f,  2.938315f,
  -0.307602f, -0.503353f, -0.080725f, -0.473909f, -0.417162f, 0.457089f,
  0.665153f,  -0.273210f, 0.028279f,  0.972220f,  -0.445596f, 1.756611f,
  -0.177892f, -0.091758f, 0.436661f,  -0.521506f, 0.133786f,  0.266743f,
  0.637367f,  -0.160084f, -1.396269f, 1.020841f,  -1.112971f, 0.919496f,
  -0.235883f, 0.651954f,  0.109061f,  -0.429463f, 0.740839f,  -0.962060f,
  0.299519f,  -0.386298f, 1.550231f,  2.464915f,  1.311969f,  2.561612f,
};

static const float vp9_var_part_nn_bias_32_layer0[8] = {
  0.368242f, 0.736617f, 0.000000f,  0.757287f,
  0.000000f, 0.613248f, -0.776390f, 0.928497f,
};

static const float vp9_var_part_nn_weights_32_layer1[8] = {
  0.939884f, -2.420850f, -0.410489f, -0.186690f,
  0.063287f, -0.522011f, 0.484527f,  -0.639625f,
};

static const float vp9_var_part_nn_bias_32_layer1[1] = {
  -0.6455006f,
};

static const NN_CONFIG vp9_var_part_nnconfig_32 = {
  FEATURES,  // num_inputs
  1,         // num_outputs
  1,         // num_hidden_layers
  {
      8,
  },  // num_hidden_nodes
  {
      vp9_var_part_nn_weights_32_layer0,
      vp9_var_part_nn_weights_32_layer1,
  },
  {
      vp9_var_part_nn_bias_32_layer0,
      vp9_var_part_nn_bias_32_layer1,
  },
};

static const float vp9_var_part_nn_weights_16_layer0[FEATURES * 8] = {
  0.742567f,  -0.580624f, -0.244528f, 0.331661f,  -0.113949f, -0.559295f,
  -0.386061f, 0.438653f,  1.467463f,  0.211589f,  0.513972f,  1.067855f,
  -0.876679f, 0.088560f,  -0.687483f, -0.380304f, -0.016412f, 0.146380f,
  0.015318f,  0.000351f,  -2.764887f, 3.269717f,  2.752428f,  -2.236754f,
  0.561539f,  -0.852050f, -0.084667f, 0.202057f,  0.197049f,  0.364922f,
  -0.463801f, 0.431790f,  1.872096f,  -0.091887f, -0.055034f, 2.443492f,
  -0.156958f, -0.189571f, -0.542424f, -0.589804f, -0.354422f, 0.401605f,
  0.642021f,  -0.875117f, 2.040794f,  1.921070f,  1.792413f,  1.839727f,
};

static const float vp9_var_part_nn_bias_16_layer0[8] = {
  2.901234f, -1.940932f, -0.198970f, -0.406524f,
  0.059422f, -1.879207f, -0.232340f, 2.979821f,
};

static const float vp9_var_part_nn_weights_16_layer1[8] = {
  -0.528731f, 0.375234f, -0.088422f, 0.668629f,
  0.870449f,  0.578735f, 0.546103f,  -1.957207f,
};

static const float vp9_var_part_nn_bias_16_layer1[1] = {
  -1.95769405f,
};

static const NN_CONFIG vp9_var_part_nnconfig_16 = {
  FEATURES,  // num_inputs
  1,         // num_outputs
  1,         // num_hidden_layers
  {
      8,
  },  // num_hidden_nodes
  {
      vp9_var_part_nn_weights_16_layer0,
      vp9_var_part_nn_weights_16_layer1,
  },
  {
      vp9_var_part_nn_bias_16_layer0,
      vp9_var_part_nn_bias_16_layer1,
  },
};
#undef FEATURES
#endif  // CONFIG_ML_VAR_PARTITION

#define FEATURES 12
#define LABELS 1
#define NODES 8
static const float vp9_part_split_nn_weights_64_layer0[FEATURES * NODES] = {
  -0.609728f, -0.409099f, -0.472449f, 0.183769f,  -0.457740f, 0.081089f,
  0.171003f,  0.578696f,  -0.019043f, -0.856142f, 0.557369f,  -1.779424f,
  -0.274044f, -0.320632f, -0.392531f, -0.359462f, -0.404106f, -0.288357f,
  0.200620f,  0.038013f,  -0.430093f, 0.235083f,  -0.487442f, 0.424814f,
  -0.232758f, -0.442943f, 0.229397f,  -0.540301f, -0.648421f, -0.649747f,
  -0.171638f, 0.603824f,  0.468497f,  -0.421580f, 0.178840f,  -0.533838f,
  -0.029471f, -0.076296f, 0.197426f,  -0.187908f, -0.003950f, -0.065740f,
  0.085165f,  -0.039674f, -5.640702f, 1.909538f,  -1.434604f, 3.294606f,
  -0.788812f, 0.196864f,  0.057012f,  -0.019757f, 0.336233f,  0.075378f,
  0.081503f,  0.491864f,  -1.899470f, -1.764173f, -1.888137f, -1.762343f,
  0.845542f,  0.202285f,  0.381948f,  -0.150996f, 0.556893f,  -0.305354f,
  0.561482f,  -0.021974f, -0.703117f, 0.268638f,  -0.665736f, 1.191005f,
  -0.081568f, -0.115653f, 0.272029f,  -0.140074f, 0.072683f,  0.092651f,
  -0.472287f, -0.055790f, -0.434425f, 0.352055f,  0.048246f,  0.372865f,
  0.111499f,  -0.338304f, 0.739133f,  0.156519f,  -0.594644f, 0.137295f,
  0.613350f,  -0.165102f, -1.003731f, 0.043070f,  -0.887896f, -0.174202f,
};

static const float vp9_part_split_nn_bias_64_layer0[NODES] = {
  1.182714f,  0.000000f,  0.902019f,  0.953115f,
  -1.372486f, -1.288740f, -0.155144f, -3.041362f,
};

static const float vp9_part_split_nn_weights_64_layer1[NODES * LABELS] = {
  0.841214f,  0.456016f,  0.869270f, 1.692999f,
  -1.700494f, -0.911761f, 0.030111f, -1.447548f,
};

static const float vp9_part_split_nn_bias_64_layer1[LABELS] = {
  1.17782545f,
};

static const NN_CONFIG vp9_part_split_nnconfig_64 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_part_split_nn_weights_64_layer0,
      vp9_part_split_nn_weights_64_layer1,
  },
  {
      vp9_part_split_nn_bias_64_layer0,
      vp9_part_split_nn_bias_64_layer1,
  },
};

static const float vp9_part_split_nn_weights_32_layer0[FEATURES * NODES] = {
  -0.105488f, -0.218662f, 0.010980f,  -0.226979f, 0.028076f,  0.743430f,
  0.789266f,  0.031907f,  -1.464200f, 0.222336f,  -1.068493f, -0.052712f,
  -0.176181f, -0.102654f, -0.973932f, -0.182637f, -0.198000f, 0.335977f,
  0.271346f,  0.133005f,  1.674203f,  0.689567f,  0.657133f,  0.283524f,
  0.115529f,  0.738327f,  0.317184f,  -0.179736f, 0.403691f,  0.679350f,
  0.048925f,  0.271338f,  -1.538921f, -0.900737f, -1.377845f, 0.084245f,
  0.803122f,  -0.107806f, 0.103045f,  -0.023335f, -0.098116f, -0.127809f,
  0.037665f,  -0.523225f, 1.622185f,  1.903999f,  1.358889f,  1.680785f,
  0.027743f,  0.117906f,  -0.158810f, 0.057775f,  0.168257f,  0.062414f,
  0.086228f,  -0.087381f, -3.066082f, 3.021855f,  -4.092155f, 2.550104f,
  -0.230022f, -0.207445f, -0.000347f, 0.034042f,  0.097057f,  0.220088f,
  -0.228841f, -0.029405f, -1.507174f, -1.455184f, 2.624904f,  2.643355f,
  0.319912f,  0.585531f,  -1.018225f, -0.699606f, 1.026490f,  0.169952f,
  -0.093579f, -0.142352f, -0.107256f, 0.059598f,  0.043190f,  0.507543f,
  -0.138617f, 0.030197f,  0.059574f,  -0.634051f, -0.586724f, -0.148020f,
  -0.334380f, 0.459547f,  1.620600f,  0.496850f,  0.639480f,  -0.465715f,
};

static const float vp9_part_split_nn_bias_32_layer0[NODES] = {
  -1.125885f, 0.753197f, -0.825808f, 0.004839f,
  0.583920f,  0.718062f, 0.976741f,  0.796188f,
};

static const float vp9_part_split_nn_weights_32_layer1[NODES * LABELS] = {
  -0.458745f, 0.724624f, -0.479720f, -2.199872f,
  1.162661f,  1.194153f, -0.716896f, 0.824080f,
};

static const float vp9_part_split_nn_bias_32_layer1[LABELS] = {
  0.71644074f,
};

static const NN_CONFIG vp9_part_split_nnconfig_32 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_part_split_nn_weights_32_layer0,
      vp9_part_split_nn_weights_32_layer1,
  },
  {
      vp9_part_split_nn_bias_32_layer0,
      vp9_part_split_nn_bias_32_layer1,
  },
};

static const float vp9_part_split_nn_weights_16_layer0[FEATURES * NODES] = {
  -0.003629f, -0.046852f, 0.220428f,  -0.033042f, 0.049365f,  0.112818f,
  -0.306149f, -0.005872f, 1.066947f,  -2.290226f, 2.159505f,  -0.618714f,
  -0.213294f, 0.451372f,  -0.199459f, 0.223730f,  -0.321709f, 0.063364f,
  0.148704f,  -0.293371f, 0.077225f,  -0.421947f, -0.515543f, -0.240975f,
  -0.418516f, 1.036523f,  -0.009165f, 0.032484f,  1.086549f,  0.220322f,
  -0.247585f, -0.221232f, -0.225050f, 0.993051f,  0.285907f,  1.308846f,
  0.707456f,  0.335152f,  0.234556f,  0.264590f,  -0.078033f, 0.542226f,
  0.057777f,  0.163471f,  0.039245f,  -0.725960f, 0.963780f,  -0.972001f,
  0.252237f,  -0.192745f, -0.836571f, -0.460539f, -0.528713f, -0.160198f,
  -0.621108f, 0.486405f,  -0.221923f, 1.519426f,  -0.857871f, 0.411595f,
  0.947188f,  0.203339f,  0.174526f,  0.016382f,  0.256879f,  0.049818f,
  0.057836f,  -0.659096f, 0.459894f,  0.174695f,  0.379359f,  0.062530f,
  -0.210201f, -0.355788f, -0.208432f, -0.401723f, -0.115373f, 0.191336f,
  -0.109342f, 0.002455f,  -0.078746f, -0.391871f, 0.149892f,  -0.239615f,
  -0.520709f, 0.118568f,  -0.437975f, 0.118116f,  -0.565426f, -0.206446f,
  0.113407f,  0.558894f,  0.534627f,  1.154350f,  -0.116833f, 1.723311f,
};

static const float vp9_part_split_nn_bias_16_layer0[NODES] = {
  0.013109f,  -0.034341f, 0.679845f,  -0.035781f,
  -0.104183f, 0.098055f,  -0.041130f, 0.160107f,
};

static const float vp9_part_split_nn_weights_16_layer1[NODES * LABELS] = {
  1.499564f, -0.403259f, 1.366532f, -0.469868f,
  0.482227f, -2.076697f, 0.527691f, 0.540495f,
};

static const float vp9_part_split_nn_bias_16_layer1[LABELS] = {
  0.01134653f,
};

static const NN_CONFIG vp9_part_split_nnconfig_16 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_part_split_nn_weights_16_layer0,
      vp9_part_split_nn_weights_16_layer1,
  },
  {
      vp9_part_split_nn_bias_16_layer0,
      vp9_part_split_nn_bias_16_layer1,
  },
};

static const float vp9_part_split_nn_weights_8_layer0[FEATURES * NODES] = {
  -0.668875f, -0.159078f, -0.062663f, -0.483785f, -0.146814f, -0.608975f,
  -0.589145f, 0.203704f,  -0.051007f, -0.113769f, -0.477511f, -0.122603f,
  -1.329890f, 1.403386f,  0.199636f,  -0.161139f, 2.182090f,  -0.014307f,
  0.015755f,  -0.208468f, 0.884353f,  0.815920f,  0.632464f,  0.838225f,
  1.369483f,  -0.029068f, 0.570213f,  -0.573546f, 0.029617f,  0.562054f,
  -0.653093f, -0.211910f, -0.661013f, -0.384418f, -0.574038f, -0.510069f,
  0.173047f,  -0.274231f, -1.044008f, -0.422040f, -0.810296f, 0.144069f,
  -0.406704f, 0.411230f,  -0.144023f, 0.745651f,  -0.595091f, 0.111787f,
  0.840651f,  0.030123f,  -0.242155f, 0.101486f,  -0.017889f, -0.254467f,
  -0.285407f, -0.076675f, -0.549542f, -0.013544f, -0.686566f, -0.755150f,
  1.623949f,  -0.286369f, 0.170976f,  0.016442f,  -0.598353f, -0.038540f,
  0.202597f,  -0.933582f, 0.599510f,  0.362273f,  0.577722f,  0.477603f,
  0.767097f,  0.431532f,  0.457034f,  0.223279f,  0.381349f,  0.033777f,
  0.423923f,  -0.664762f, 0.385662f,  0.075744f,  0.182681f,  0.024118f,
  0.319408f,  -0.528864f, 0.976537f,  -0.305971f, -0.189380f, -0.241689f,
  -1.318092f, 0.088647f,  -0.109030f, -0.945654f, 1.082797f,  0.184564f,
};

static const float vp9_part_split_nn_bias_8_layer0[NODES] = {
  -0.237472f, 2.051396f,  0.297062f, -0.730194f,
  0.060472f,  -0.565959f, 0.560869f, -0.395448f,
};

static const float vp9_part_split_nn_weights_8_layer1[NODES * LABELS] = {
  0.568121f,  1.575915f,  -0.544309f, 0.751595f,
  -0.117911f, -1.340730f, -0.739671f, 0.661216f,
};

static const float vp9_part_split_nn_bias_8_layer1[LABELS] = {
  -0.63375306f,
};

static const NN_CONFIG vp9_part_split_nnconfig_8 = {
  FEATURES,  // num_inputs
  LABELS,    // num_outputs
  1,         // num_hidden_layers
  {
      NODES,
  },  // num_hidden_nodes
  {
      vp9_part_split_nn_weights_8_layer0,
      vp9_part_split_nn_weights_8_layer1,
  },
  {
      vp9_part_split_nn_bias_8_layer0,
      vp9_part_split_nn_bias_8_layer1,
  },
};
#undef NODES
#undef FEATURES
#undef LABELS

// Partition pruning model(linear).
static const float vp9_partition_feature_mean[24] = {
  303501.697372f, 3042630.372158f, 24.694696f, 1.392182f,
  689.413511f,    162.027012f,     1.478213f,  0.0,
  135382.260230f, 912738.513263f,  28.845217f, 1.515230f,
  544.158492f,    131.807995f,     1.436863f,  0.0f,
  43682.377587f,  208131.711766f,  28.084737f, 1.356677f,
  138.254122f,    119.522553f,     1.252322f,  0.0f,
};

static const float vp9_partition_feature_std[24] = {
  673689.212982f, 5996652.516628f, 0.024449f, 1.989792f,
  985.880847f,    0.014638f,       2.001898f, 0.0f,
  208798.775332f, 1812548.443284f, 0.018693f, 1.838009f,
  396.986910f,    0.015657f,       1.332541f, 0.0f,
  55888.847031f,  448587.962714f,  0.017900f, 1.904776f,
  98.652832f,     0.016598f,       1.320992f, 0.0f,
};

// Error tolerance: 0.01%-0.0.05%-0.1%
static const float vp9_partition_linear_weights[24] = {
  0.111736f, 0.289977f, 0.042219f, 0.204765f, 0.120410f, -0.143863f,
  0.282376f, 0.847811f, 0.637161f, 0.131570f, 0.018636f, 0.202134f,
  0.112797f, 0.028162f, 0.182450f, 1.124367f, 0.386133f, 0.083700f,
  0.050028f, 0.150873f, 0.061119f, 0.109318f, 0.127255f, 0.625211f,
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VP9_ENCODER_VP9_PARTITION_MODELS_H_
