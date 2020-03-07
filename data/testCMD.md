docker build -t todaylg/envtools ./

docker run -t -i todaylg/envtools /bin/bash

docker ps -a

docker container ls -a

docker container start/stop containerID

docker exec -it containerID bash

docker container prune
