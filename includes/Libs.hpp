/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Libs.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dmartiro <dmartiro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 06:37:27 by dmartiro          #+#    #+#             */
/*   Updated: 2023/12/03 23:15:55 by dmartiro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBS_HPP
#define LIBS_HPP
#include <iostream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <limits.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <list>
#include <fstream>
#include <sys/select.h>
#include <map>
#include <iterator>
#include <stack>
#include <vector>
#include "DefaultSetup.hpp"

typedef std::vector<std::string> methods_t;
typedef std::vector<std::string> indexes_t;
#endif