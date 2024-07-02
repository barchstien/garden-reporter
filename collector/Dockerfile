FROM python:3.10-slim

ENV TZ="Europe/Paris"
RUN date

ENV PROJECT_DIR /home/garden
RUN echo ${PROJECT_DIR}
RUN mkdir -p ${PROJECT_DIR}
RUN useradd garden -d ${PROJECT_DIR}

WORKDIR ${PROJECT_DIR}

# copy source files
COPY Pipfile Pipfile.lock collector.yaml water-web-config.yaml ${PROJECT_DIR}/
RUN mkdir -p ${PROJECT_DIR}/source
COPY source/*.py ${PROJECT_DIR}/source/
RUN mkdir -p ${PROJECT_DIR}/source/static
COPY source/static/* ${PROJECT_DIR}/source/static/
RUN chown -R garden:garden ${PROJECT_DIR}

# setup config folder permission
ARG WATER_WEB_CONTROL_CONFIG_FOLDER=/var/lib/water-web-control
RUN mkdir -p $WATER_WEB_CONTROL_CONFIG_FOLDER
RUN chown -R garden:garden $WATER_WEB_CONTROL_CONFIG_FOLDER

USER garden

RUN pip install pipenv
ENV PATH="${PATH}:${PROJECT_DIR}/.local/bin"

RUN mkdir -p ${PROJECT_DIR}/.venv
RUN pipenv install --system --deploy --ignore-pipfile

CMD ["python", "-u", "./source/collector.py"]

