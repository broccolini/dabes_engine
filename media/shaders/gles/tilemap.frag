precision mediump float;
uniform vec2 tileSize;
uniform vec2 sheetRowsCols;
uniform vec2 mapRowsCols;
varying vec4 textureVarying;
uniform sampler2D atlas;
uniform sampler2D tileset;

void main()
{
  vec2 tilePos = (floor(textureVarying.xy * mapRowsCols) + vec2(0.5, 0.5)) / mapRowsCols;
  vec4 tileVal = texture2D(atlas, tilePos);
  float cell = tileVal.r;

  mediump float index = ceil(cell * 256.0);
  vec2 baseTilePos = floor(vec2(mod(index, sheetRowsCols.x),
                                index / sheetRowsCols.y)) / sheetRowsCols;
  
  vec2 perMap = vec2(1.0, 1.0) / mapRowsCols;
  vec2 internalPos = mod(textureVarying.xy, perMap) / (perMap * sheetRowsCols);
  vec4 tileColor = texture2D(tileset, baseTilePos + internalPos);
  
  tileColor.a = tileColor.a * (1.0 - tileVal.a);
  gl_FragColor = vec4(0.0,0.0,0.0,1.0 - tileVal.a);
}