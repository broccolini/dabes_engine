precision mediump float;
uniform vec2 tileSize;
uniform vec2 sheetRowsCols;
uniform vec2 mapRowsCols;
varying vec4 textureVarying;
uniform sampler2D atlas;
uniform sampler2D tileset;

void main()
{
  //vec2 tilePos = textureVarying.xy / tileSize.xy;
  vec2 tilePos = textureVarying.xy;
  vec4 tileVal = texture2D(atlas, tilePos);
  float cell = tileVal.r;
  if (tileVal.a > 0.0) {
    gl_FragColor = vec4(0.0,0.0,0.0,0.0);
    return;
  }
  gl_FragColor = vec4(tilePos,0.0,1.0);
  //gl_FragColor = texture2D(tileset, vec2(0.1,0.1));
  return;
  
  //float index = floor(texture2D(atlas, tilePos).r * 256.0);
  mediump float index = 255.0;
  vec2 sheetCell = 1.0 / sheetRowsCols;
  vec2 baseTilePos = sheetCell * floor(vec2(mod(index, sheetRowsCols.x),
                                            index / sheetRowsCols.y));
  vec2 internalPos = sheetCell * vec2(mod(textureVarying.x * tileSize.x, 1.0),
                                      mod(textureVarying.y * tileSize.y, 1.0));
  gl_FragColor = texture2D(tileset, baseTilePos + internalPos);
  //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}