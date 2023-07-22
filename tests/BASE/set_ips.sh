#!/bin/bash

# Verifico si se recibieron 4 argumentos
if [ "$#" -ne 4 ]; then
  echo "Se requieren 4 IPs como argumentos. Se reciben en orden: kernel, cpu, fs, memoria"
  exit 1
fi

# Argumentos recibidos por parametro
ip_memoria="$1"
ip_cpu="$2"
ip_fs="$3"
ip_kernel="$4"

# Array con los nombres de directorio
modulos=("BASE" "DEADLOCK" "ERROR" "FILESYSTEM" "MEMORIA")

# Array con los nombres de los archivos de configuración
configs=("cpu" "kernel" "memoria" "filesystem")

# Recorro los directorios
for mod in "${modulos[@]}"; do
  config_mod="$mod/configs"

  if [ "$mod" == "consola" ]; then
    sed -i "s/IP_KERNEL=.*/IP_KERNEL="$ip_kernel"/" "$mod/consola.config"
  else
    # Recorro los archivos de configuracion en el directorio
    for config in "${configs[@]}"; do
      config_file="$config_mod/${mod}_$config.config"

      # Actualizo las lineas en el archivo de configuracion
      case $mod in
        "kernel")
          sed -i "s/IP_KERNEL=.*/IP_KERNEL="$ip_kernel"/; s/IP_MEMORIA=.*/IP_MEMORIA="$ip_memoria"/; s/IP_FILESYSTEM=.*/IP_FILESYSTEM="$ip_fs"/; s/IP_CPU=.*/IP_CPU="$ip_cpu"/" "$config_file"
          ;;
        "cpu")
          sed -i "s/IP_CPU=.*/IP_CPU="$ip_cpu"/; s/IP_MEMORIA=.*/IP_MEMORIA="$ip_memoria"/" "$config_file"
          ;;
        "fileSystem")
          sed -i "s/IP_FILESYSTEM=.*/IP_FILESYSTEM="$ip_fs"/; s/IP_MEMORIA=.*/IP_MEMORIA="$ip_memoria"/" "$config_file"
          ;;
        "memoria")
          sed -i "s/IP_MEMORIA=.*/IP_MEMORIA="$ip_memoria"/" "$config_file"
          ;;
        *)
          echo "Modulo no reconocido: $mod"
          ;;
      esac

      echo "IP actualizada en $config_file"
    done
  fi
  echo "Configs de $mod actualizados."
done
