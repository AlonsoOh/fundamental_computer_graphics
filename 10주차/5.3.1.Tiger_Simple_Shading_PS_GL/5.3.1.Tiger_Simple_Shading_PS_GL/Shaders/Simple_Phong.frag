#version 330

// Modified from a shader in OpenGL 4 Shading Language Cookbook - Third Edition by D. Wolff
uniform struct LightInfo {
  vec4 Position;  // Light position in eye coords.
  vec3 La;         // Ambient light intesity
  vec3 L;          // Diffuse and specular light intensity
} Light;

uniform struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
} Material;

uniform bool use_halfway_vector;

in vec3 Position;
in vec3 Normal;
layout( location = 0 ) out vec4 FragColor;

vec3 phongModel( vec3 position, vec3 n ) {  
  vec3 ambient = Light.La * Material.Ka;
  vec3 s = normalize( Light.Position.xyz - position );
  float sDotN = max( dot(s,n), 0.0 );
  vec3 diffuse = Material.Kd * sDotN;

  vec3 spec = vec3(0.0);
  if( sDotN > 0.0 ) {
    vec3 v = normalize(-position.xyz);
    if (use_halfway_vector) {
       vec3 h = normalize(s + v);
       spec = Material.Ks *
                     pow( max( dot(n, h), 0.0 ), Material.Shininess );
    }
    else {
        vec3 r = reflect( -s, n );
        spec = Material.Ks *
                     pow( max( dot(r, v), 0.0 ), Material.Shininess );
     }
  }
  return ambient + Light.L * (diffuse + spec);
}

void main() {
    FragColor = vec4(phongModel(Position, normalize(Normal)), 1.0);
}



