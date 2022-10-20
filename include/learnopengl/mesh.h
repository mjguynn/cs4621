#pragma once
#include <vector>
#include "../terrapainter/math.h"

#include <glad/gl.h>

using namespace std;

struct Vertex
{
  vec3 Position;
  // normal
  vec3 Normal;
};

class Mesh
{
public:
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  unsigned int mNumTrisPerStrip;
  unsigned int mNumStrips;
  unsigned int VAO;

  Mesh(vector<Vertex> vertices, vector<unsigned int> indices, unsigned int numTrisPerStrip, unsigned int numStrips)
  {
    this->vertices = vertices;
    this->indices = indices;
    this->mNumTrisPerStrip = numTrisPerStrip;
    this->mNumStrips = numStrips;
    setupMesh();
  }
  /*~Mesh() noexcept {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);
  }*/
  void Draw(GLenum mode)
  {
    // TODO
  }

  void DrawStrips()
  {
    glBindVertexArray(VAO);
    for (unsigned int strip = 0; strip < mNumStrips; strip++)
    {
      glDrawElements(GL_TRIANGLE_STRIP, mNumTrisPerStrip + 2, GL_UNSIGNED_INT, (void *)(sizeof(unsigned int) * (mNumTrisPerStrip + 2) * strip));
    }
    glBindVertexArray(0);
  }

private:
  unsigned int VBO, EBO;

  void setupMesh()
  {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // normal Positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    glBindVertexArray(0);
  }
};
