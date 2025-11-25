#include "objloader.hpp"
#include <cstring>
#include <cstdio>
#include <string>

// função que lê ficheiro .obj e devolve listas de vértices, uvs e normais
bool loadOBJ(
    const char *path,
    std::vector<glm::vec3> &out_vertices,
    std::vector<glm::vec2> &out_uvs,
    std::vector<glm::vec3> &out_normals)
{
    // listas temporárias (armazenam tudo que for lido)
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    // abrir ficheiro pelo path dado
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Impossible to open the file ! (%s)\n", path);
        return false;
    }

    // ler até ao fim do ficheiro
    while (1)
    {
        char lineHeader[256]; // assumir que a palavra nao tem mais de 256 chars

        // ler a primeira palavra da linha
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File → sai do loop

        // analisar o que a primeira palavra significa
        // se for "v", a linha descreve um vértice (x y z)
        if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }

        // se for "vt", descreve coordenadas de textura (u v)
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }

        // se for "vn", descreve uma normal (x y z)
        else if (strcmp(lineHeader, "vn") == 0)
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }

        // se for "f", descreve uma face (índices dos vértices/uvs/normais)
        else if (strcmp(lineHeader, "f") == 0)
        {
            // read the rest of the line into a buffer
            char line[1024];
            if (!fgets(line, sizeof(line), file)) break;
            std::istringstream ss(line);
            std::string token;

            // collect face indices (support v, v/vt, v//vn, v/vt/vn)
            std::vector<unsigned int> face_v, face_vt, face_vn;
            while (ss >> token) {
                unsigned int vi = 0, ti = 0, ni = 0;
                if (sscanf(token.c_str(), "%u/%u/%u", &vi, &ti, &ni) == 3) {
                    face_v.push_back(vi);
                    face_vt.push_back(ti);
                    face_vn.push_back(ni);
                } else if (sscanf(token.c_str(), "%u//%u", &vi, &ni) == 2) {
                    face_v.push_back(vi);
                    face_vt.push_back(0);
                    face_vn.push_back(ni);
                } else if (sscanf(token.c_str(), "%u/%u", &vi, &ti) == 2) {
                    face_v.push_back(vi);
                    face_vt.push_back(ti);
                    face_vn.push_back(0);
                } else if (sscanf(token.c_str(), "%u", &vi) == 1) {
                    face_v.push_back(vi);
                    face_vt.push_back(0);
                    face_vn.push_back(0);
                }
            }

            if (face_v.size() < 3) {
                // malformed face
                fclose(file);
                return false;
            }

            // triangulate polygon (fan triangulation)
            for (size_t i = 1; i + 1 < face_v.size(); ++i) {
                vertexIndices.push_back(face_v[0]); vertexIndices.push_back(face_v[i]); vertexIndices.push_back(face_v[i+1]);
                uvIndices.push_back(face_vt[0]); uvIndices.push_back(face_vt[i]); uvIndices.push_back(face_vt[i+1]);
                normalIndices.push_back(face_vn[0]); normalIndices.push_back(face_vn[i]); normalIndices.push_back(face_vn[i+1]);
            }
        }
    }

    // até agora → só tínhamos listas de índices e dados separados
    // agora vamos juntar tudo numa forma que o OpenGL consiga usar diretamente

    // para cada vértice de cada triângulo (linhas "f")
    for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[vertexIndex - 1]; // -1 porque .obj começa em 1
        out_vertices.push_back(vertex);

        unsigned int uvIndex = uvIndices[i];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        out_uvs.push_back(uv);

        unsigned int normalIndex = normalIndices[i];
        glm::vec3 normal = temp_normals[normalIndex - 1];
        out_normals.push_back(normal);
    }

    // fechar ficheiro
    fclose(file);

    // sucesso
    return true;
}
