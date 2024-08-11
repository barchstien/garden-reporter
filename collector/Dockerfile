FROM python:3.10-slim

ENV TZ="Europe/Paris"
RUN date

ENV PROJECT_DIR /home/garden
RUN echo ${PROJECT_DIR}
RUN mkdir -p ${PROJECT_DIR}
RUN useradd garden -d ${PROJECT_DIR}

WORKDIR ${PROJECT_DIR}

# copy source files
# collector.yaml water-web-config.yaml 
COPY Pipfile Pipfile.lock ${PROJECT_DIR}/
RUN mkdir -p ${PROJECT_DIR}/source
COPY source/*.py ${PROJECT_DIR}/source/
RUN mkdir -p ${PROJECT_DIR}/source/static
COPY source/static/* ${PROJECT_DIR}/source/static/
RUN chown -R garden:garden ${PROJECT_DIR}

# setup config folder permission
ARG CONFIG_FOLDER=/var/lib/garden-collector
RUN mkdir -p $CONFIG_FOLDER
RUN chown -R garden:garden $CONFIG_FOLDER

USER garden

RUN pip install pipenv
ENV PATH="${PATH}:${PROJECT_DIR}/.local/bin"

RUN mkdir -p ${PROJECT_DIR}/.venv
RUN pipenv install --system --deploy --ignore-pipfile

CMD ["python", "-u", "./source/collector.py"]

