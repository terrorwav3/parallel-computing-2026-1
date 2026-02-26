inicializar_grilla(N, M, n_peces, n_tiburones)

para t = 1 hasta T:
    para cada pez en grilla (orden aleatorio):
        vecinas_vacias = obtener_vecinas_vacias(pez)
        si vecinas_vacias no está vacío:
            mover(pez, elegir_al_azar(vecinas_vacias))
            si pez.edad >= fish_breed:
                crear_cria(posicion_anterior)
                pez.edad = 0

    para cada tiburón en grilla (orden aleatorio):
        vecinas_con_pez = obtener_vecinas_con_pez(tiburón)
        si vecinas_con_pez no está vacío:
            comer(tiburón, elegir_al_azar(vecinas_con_pez))
        sino:
            vecinas_vacias = obtener_vecinas_vacias(tiburón)
            si vecinas_vacias no está vacío:
                mover(tiburón, elegir_al_azar(vecinas_vacias))
            tiburón.energía -= 1
        si tiburón.energía <= 0:
            eliminar(tiburón)
        si tiburón.edad >= shark_breed:
            crear_cria(posicion_anterior)
            tiburón.edad = 0

    registrar(contar_peces(), contar_tiburones())
